/*
***************************************************************************  
**
**  File    : I2C_ADW0720.h
**  Version : v1.2  - 28-07-2020
**
**  Copyright (c) 2020 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/


#ifndef _I2C_ADW0720_H
#define _I2C_ADW0720_H
#include "Arduino.h"
#include "Wire.h"

#define I2C_SLAVE_ADDRESS 0x18
#define _WRITEDELAY 50   // 10
#define _READDELAY  20   // 5

// status bits
enum  {  SLT_PRESSED_BIT, SLT_QUICKRELEASE_BIT, SLT_MIDRELEASE_BIT, SLT_LONGRELEASE_BIT
       , SLT_HIGH_BIT, SLT_TOGGLE_BIT, SLT_PULSE_BIT };
enum  {  CMD_READCONF, CMD_WRITECONF, CMD_DUM2, CMD_DUM3, CMD_DUM4
       , CMD_DUM5, CMD_DUM6,  CMD_REBOOT };


// Map to the various registers on the I2C Rotary Encoder
enum boardRegisters {
  I2CADW0720_ADDRESS        = 0x00,
  I2CADW0720_MAJORRELEASE   = 0x01,
  I2CADW0720_MINORRELEASE   = 0x02,
  I2CADW0720_SYSSTATUS      = 0x03,
  I2CADW0720_SLOTSTATUS     = 0x04, // SLOT_STATUS_0!
  I2CADW0720_SLOTSTATUS_1   = 0x05,
  I2CADW0720_SLOTSTATUS_2   = 0x06,
  I2CADW0720_SLOTSTATUS_3   = 0x07,
  I2CADW0720_SLOTSTATUS_4   = 0x08,
  I2CADW0720_SLOTSTATUS_5   = 0x09,
  I2CADW0720_SLOTSTATUS_6   = 0x0A,
  I2CADW0720_SLOTSTATUS_7   = 0x0B,
  I2CADW0720_SLOT_MODES     = 0x0C,
  I2CADW0720_DEBOUNCETIME   = 0x0D,  // microSeconds
  I2CADW0720_MIDPRESSTIME   = 0x0E,  // 2 bytes 0x0E 0x0F
  I2CADW0720_LONGPRESSTIME  = 0x10,  // 2 bytes 0x10 0x11
  I2CADW0720_SLOT_NR        = 0x12,
  I2CADW0720_SLOT_FUNC      = 0x13,
  I2CADW0720_SLOT_LOWTIME   = 0x14, // 2 bytes 0x14 0x15
  I2CADW0720_SLOT_HIGHTIME  = 0x16, // 2 bytes 0x16 0x17
  I2CADW0720_SLOT_DURATION  = 0x18, // 2 bytes 0x18 0x19
  I2CADW0720_MODESETTINGS   = 0x1A,
  //----
  I2CADW0720_COMMAND        = 0xF0   // -> this is NOT a "real" register!!
};

class I2CADW0720
{

public:
  I2CADW0720();

  bool       begin(TwoWire &wireBus = Wire, uint8_t deviceAddress = I2C_SLAVE_ADDRESS);
  bool       isConnected();

//-------------------------------------------------------------------------------------
//-------------------------- GETTERS --------------------------------------------------
//-------------------------------------------------------------------------------------
  uint8_t   getSysStatus();               // reads the status byte
  uint8_t   getSlotStatus(uint8_t);       // reads the slot4 status byte
  uint8_t   getSlotModes();
  int8_t    getWhoAmI();                  // read the Address Register
  uint8_t   getDebounceTime();            // read the Debounce Time of the switch (5 .. 250 micro seconds)
  uint16_t  getMidPressTime();            // read the Mid Press Time of the switch (100 .. 5000 milli seconds)
  uint16_t  getLongPressTime();           // read the Long Press Time of the switch (300 .. 10000 milli seconds)
  uint8_t   getMajorRelease();            // read the Major Firmware Release byte (0 .. 255)
  uint8_t   getMinorRelease();            // read the Minor Firmware Release byte (0 .. 255)
  uint8_t   getModeSettings();            // read the Mode register byte (0 .. 255)
  bool      getModeSettings(uint8_t);     // read a Mode register Bit (true if set/1 otherwise false/0)

//-------------------------------------------------------------------------------------
//-------------------------- SETTERS --------------------------------------------------
//-------------------------------------------------------------------------------------
  bool      setModeOutput(uint8_t);       // set slot as OUTPUT
  bool      setModeInput(uint8_t);        // set slot as INPUT
  // set <led> HIGH/LOW> <Duration>
  bool      setOutputToggle(uint8_t, bool, uint16_t);
  // set blink <led>, <HIGH time>, <LOW time>, <Duration>  
  bool      setOutputPulse(uint8_t, uint16_t, uint16_t, uint16_t);         
  bool      setDebounceTime(uint8_t);     // set the Debounce Time of the switch (5 .. 250 micro seconds)
  bool      setMidPressTime(uint16_t);    // set the Mid Press Time of the switch (100 .. 5000 milli seconds)
  bool      setLongPressTime(uint16_t);   // set the Long Press Time of the switch (300 .. 10000 milli seconds)
  bool      writeCommand(uint8_t);        // write a command to the Slave (CMD_READCONF | CMD_WRITECONF | CMD_REBOOT)

  bool      setI2Caddress(uint8_t newAddress);   // set a new I2C address for this Slave (1 .. 127)        
  
//-------------------------------------------------------------------------------------
//-------------------------- HELPERS --------------------------------------------------
//-------------------------------------------------------------------------------------
  bool      isSlotPressed(uint8_t);            // true if the Button is pressed
  bool      isSlotQuickReleased(uint8_t);      // true if the Button is released before midPressTime
  bool      isSlotMidReleased(uint8_t);        // true if the Button is released between midPressTime and longPressTime
  bool      isSlotLongReleased(uint8_t);       // true if the Button is released after longPressTime

  void printRegister(HardwareSerial *serOut, size_t const size, void const * const ptr);

private:
  TwoWire             *_I2Cbus;
  uint8_t             _I2Caddress;
  uint32_t            _readTimer;
  volatile uint8_t   _SYSstatus;
  volatile uint8_t   _SLOTmodes;
  volatile uint8_t   _SLOTstatus[8];

  uint8_t   readReg1Byte(uint8_t reg);
  int16_t   readReg2Byte(uint8_t reg);
  int32_t   readReg4Byte(uint8_t reg);

  bool      writeReg1Byte(uint8_t reg, uint8_t val);
  bool      writeReg2Byte(uint8_t reg, int16_t val);
  bool      writeReg3Byte(uint8_t reg, int32_t val);
  bool      writeReg4Byte(uint8_t reg, int32_t val);

//static void onReceiveCallback(int numbytes);

  //void showRegister(size_t const size, void const * const ptr);
};

#endif

/***************************************************************************
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to permit
* persons to whom the Software is furnished to do so, subject to the
* following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
* OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
* THE USE OR OTHER DEALINGS IN THE SOFTWARE.
* 
***************************************************************************/
