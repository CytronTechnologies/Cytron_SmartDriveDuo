#ifndef CYTRON_SMARTDRIVEDUO_h
#define CYTRON_SMARTDRIVEDUO_h

#include "Arduino.h"
#include <SoftwareSerial.h>

// Arduino Leonardo
#if defined (__AVR_ATmega32U4__)
  #define Serial Serial1
#else
  #define Serial Serial
#endif

enum {
  RC_MCU,
  PWM_INDEPENDENT,
  PWM_MIXED,
  SERIAL_SIMPLIFIED,
  SERIAL_PACKETIZED
};

class Cytron_SmartDriveDuo
{
  public:
  	//Cytron_SmartDriveDuo(int mode, int rc1Pin, int rc2Pin); // For RC MCU mode
    Cytron_SmartDriveDuo(int mode, int in1Pin, int in2Pin, int an1Pin, int an2Pin); // For PWM Independent mode
    Cytron_SmartDriveDuo(int mode, int txPin, uint32_t baudrate); // For Serial Simplified mode
    Cytron_SmartDriveDuo(int mode, int txPin, int boardId, uint32_t baudrate); // For Serial Packetized mode
    void initialByte(uint8_t dummyByte);
    void control(signed int motorLeftSpeed, signed int motorRightSpeed);
    
  private:
    SoftwareSerial* MDDSSerial;
  	uint8_t _mode;
  	uint8_t _rc1Pin, _rc2Pin;
  	uint8_t _an1Pin, _an2Pin, _in1Pin, _in2Pin;
  	uint8_t _txPin, _rxPin = 99, _boardId;
    uint8_t headerByte = 0x85, addressByte, commandByte, checksum;
    int _motorLSpeed, _motorRSpeed;
    int rc1Delay, rc2Delay;
};

#endif
