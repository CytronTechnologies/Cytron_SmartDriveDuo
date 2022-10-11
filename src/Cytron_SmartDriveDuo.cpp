/*
Original written by:
  08/05/17  Idris, Cytron Technologies
*/

#include "Cytron_SmartDriveDuo.h"

/*
Cytron_SmartDriveDuo::Cytron_SmartDriveDuo(int mode, int rc1Pin, int rc2Pin)
{
  _mode = mode;
  _rc1Pin = rc1Pin;
  _rc2Pin = rc2Pin;

  pinMode(_rc1Pin, OUTPUT);
  pinMode(_rc2Pin, OUTPUT);
}
*/

Cytron_SmartDriveDuo::Cytron_SmartDriveDuo(int mode, int in1Pin, int in2Pin, int an1Pin, int an2Pin)
{
  _mode = mode;
  _an1Pin = an1Pin;
  _an2Pin = an2Pin;
  _in1Pin = in1Pin;
  _in2Pin = in2Pin;
  
  pinMode(_an1Pin, OUTPUT);
  pinMode(_an2Pin, OUTPUT);
  pinMode(_in1Pin, OUTPUT);
  pinMode(_in2Pin, OUTPUT);
}

SoftwareSerial* MDDSSerial;
boolean hardwareSerial;
Cytron_SmartDriveDuo::Cytron_SmartDriveDuo(int mode, int txPin, uint32_t baudrate)
{
  _mode = mode;
  _txPin = txPin;

  if (_txPin == 1) {
    hardwareSerial = true;
    Serial.begin(baudrate);
    while (!Serial);
  }
  else {
    hardwareSerial = false;
    pinMode(_txPin, OUTPUT);
    MDDSSerial = new SoftwareSerial(_rxPin, _txPin);
    MDDSSerial->begin(baudrate);
  }
}

Cytron_SmartDriveDuo::Cytron_SmartDriveDuo(int mode, int txPin, int boardId, uint32_t baudrate)
{
  _mode = mode;
  _txPin = txPin;
  _boardId = boardId;

  if (_txPin == 1) {
    hardwareSerial = true;
    Serial.begin(baudrate);
    while (!Serial);
    //delay(1000);
    //Serial.write(0x80);
  }
  else {
    hardwareSerial = false;
    pinMode(_txPin, OUTPUT);
    MDDSSerial = new SoftwareSerial(_rxPin, _txPin);
    MDDSSerial->begin(baudrate);
    //delay(1000);
    //MDDSSerial->write(0x80);
  }
}

void Cytron_SmartDriveDuo::initialByte(uint8_t dummyByte)
{
  if (hardwareSerial == true) {
    Serial.write(dummyByte);
  }
  else {
    MDDSSerial->write(dummyByte);
  }
}

void Cytron_SmartDriveDuo::control(signed int motorLSpeed, signed int motorRSpeed)
{
  switch (_mode) {
    case RC_MCU:
      if (motorLSpeed >= 0) {
        if (motorLSpeed > 100) motorLSpeed = 100;
        rc1Delay = map(motorLSpeed, 0, 100, 1500, 2480);
        digitalWrite(_rc1Pin, HIGH);
        delayMicroseconds(rc1Delay);
        digitalWrite(_rc1Pin, LOW);
      }
      else if (motorLSpeed < 0) {
        if (motorLSpeed < -100) motorLSpeed = -100;
        rc1Delay = map(motorLSpeed, -100, 0, 500, 1500);
        digitalWrite(_rc1Pin, HIGH);
        delayMicroseconds(rc1Delay);
        digitalWrite(_rc1Pin, LOW);
      }

      if (motorRSpeed >= 0) {
        if (motorRSpeed > 100) motorRSpeed = 100;
        rc2Delay = map(motorRSpeed, 0, 100, 1500, 2480);
        digitalWrite(_rc2Pin, HIGH);
        delayMicroseconds(rc2Delay);
        digitalWrite(_rc2Pin, LOW);
      }
      else if (motorRSpeed < 0) {
        if (motorRSpeed < -100) motorRSpeed = -100;
        rc2Delay = map(motorRSpeed, -100, 0, 500, 1500);
        digitalWrite(_rc2Pin, HIGH);
        delayMicroseconds(rc2Delay);
        digitalWrite(_rc2Pin, LOW);
      }
      break;

    case PWM_INDEPENDENT:
      if (motorLSpeed >= 0) {
        if (motorLSpeed > 100) motorLSpeed = 100;
        motorLSpeed = motorLSpeed * 2.55;
        analogWrite(_an1Pin, motorLSpeed);
        digitalWrite(_in1Pin, LOW);
      }
      else if (motorLSpeed < 0) {
        if (motorLSpeed < -100) motorLSpeed = -100;
        motorLSpeed = motorLSpeed * -2.55;
        analogWrite(_an1Pin, motorLSpeed);
        digitalWrite(_in1Pin, HIGH);
      }
      
      if (motorRSpeed >= 0) {
        if (motorRSpeed > 100) motorRSpeed = 100;
        motorRSpeed = motorRSpeed * 2.55;
        analogWrite(_an2Pin, motorRSpeed);
        digitalWrite(_in2Pin, HIGH);
      }
      else if (motorRSpeed < 0) {
        if (motorRSpeed < -100) motorRSpeed = -100;
        motorRSpeed = motorRSpeed * -2.55;
        analogWrite(_an2Pin, motorRSpeed);
        digitalWrite(_in2Pin, LOW);
      }
      break;

    case SERIAL_SIMPLIFIED:
      if (motorLSpeed >= 0) {
        commandByte = 0;
        _motorLSpeed = map(motorLSpeed, 0, 100, 0, 63);
      }
      else if (motorLSpeed < 0) {
        commandByte = 0x40;
        _motorLSpeed = map(motorLSpeed, 0, -100, 0, 63);
      }
      commandByte = commandByte | _motorLSpeed;
      if (hardwareSerial == true) {
        Serial.write(commandByte);
      }
      else {
        MDDSSerial->write(commandByte);
      }

      if (motorRSpeed >= 0) {
        commandByte = 0xC0;
        _motorRSpeed = map(motorRSpeed, 0, 100, 0, 63);
      }
      else if (motorRSpeed < 0) {
        commandByte = 0x80;
        _motorRSpeed = map(motorRSpeed, 0, -100, 0, 63);
      }
      commandByte = commandByte | _motorRSpeed;
      if (hardwareSerial == true) {
        Serial.write(commandByte);
      }
      else {
        MDDSSerial->write(commandByte);
      }
      break;

    case SERIAL_PACKETIZED:
      // Left motor
      addressByte = _boardId;
      commandByte = map(motorLSpeed, -100, 100, 0, 255);
      checksum = headerByte + addressByte + commandByte;
      if (hardwareSerial == true) {
        Serial.write(headerByte);
        Serial.write(addressByte);
        Serial.write(commandByte);
        Serial.write(checksum);
      }
      else {
        MDDSSerial->write(headerByte);
        MDDSSerial->write(addressByte);
        MDDSSerial->write(commandByte);
        MDDSSerial->write(checksum);
      }
      // Right motor
      addressByte = _boardId | 0b00001000;
      commandByte = map(motorRSpeed, -100, 100, 0, 255);
      checksum = headerByte + addressByte + commandByte;
      if (hardwareSerial == true) {
        Serial.write(headerByte);
        Serial.write(addressByte);
        Serial.write(commandByte);
        Serial.write(checksum);
      }
      else {
        MDDSSerial->write(headerByte);
        MDDSSerial->write(addressByte);
        MDDSSerial->write(commandByte);
        MDDSSerial->write(checksum);
      }

      break;

    default:
      break;
  }
}
