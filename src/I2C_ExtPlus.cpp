/*
***************************************************************************  
**
**  File    : I2C_ExtPlus.cpp
**  Version : v1.1
**
**  Copyright (c) 2020 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/

#include "I2C_ExtPlus.h"
#include "Arduino.h"

// Constructor
I2CEXTPL::I2CEXTPL() { }

// Initializes the I2C_LaS_Lib
// Returns false if I2C_LaS_Lib is not detected
//-------------------------------------------------------------------------------------
bool I2CEXTPL::begin(TwoWire &wireBus, uint8_t deviceAddress)
{
  _I2Cbus = &wireBus;
  _I2Cbus->begin(); 
  _I2Cbus->setClock(100000);

  _I2Caddress = deviceAddress;

  if (isConnected() == false)
    return (false); // Check for I2C_ExtPlus board

  return (true); // Everything is OK!

} // begin()

// Change the I2C address of this I2C Slave address to newAddress
//-------------------------------------------------------------------------------------
bool I2CEXTPL::setI2Caddress(uint8_t newAddress)
{
  if (writeReg1Byte(I2CEXTPL_ADDRESS, newAddress)) 
  {
    // Once the address is changed, we need to change it in the library
    //-->?? _I2Caddress = newAddress;
    return true;
  }
  return false;

} // newAddress()

//-------------------------------------------------------------------------------------
//-------------------------- SETTERS --------------------------------------------------
//-------------------------------------------------------------------------------------


// Sets specific slot as type OUTPUT
//-------------------------------------------------------------------------------------
bool I2CEXTPL::setModeOutput(uint8_t slotNr)
{
  uint8_t slotMode = readReg1Byte(I2CEXTPL_SLOT_MODES);
  printRegister(&Serial, 1, &slotMode);
  slotMode &= ~_BV(slotNr);  //-- clear bit
  delay(10);
  return(writeReg1Byte(I2CEXTPL_SLOT_MODES, slotMode));
}

// Sets specific slot as type OUTPUT
//-------------------------------------------------------------------------------------
bool I2CEXTPL::setModeInput(uint8_t slotNr)
{
  uint8_t slotMode = readReg1Byte(I2CEXTPL_SLOT_MODES);
  slotMode |= _BV(slotNr); //-- set bit 
  delay(10);
  return(writeReg1Byte(I2CEXTPL_SLOT_MODES, slotMode));
}

// Sets specific slot as type On/Off
//-------------------------------------------------------------------------------------
bool I2CEXTPL::setOutputToggle(uint8_t slotNr, bool isHigh, uint16_t duration)
{
  byte slotFunc = 0;
  if (isHigh) slotFunc |= (1<<SLT_HIGH_BIT);    // bit 0 is On (1) or off (0)
  slotFunc |= (1<<SLT_TOGGLE_BIT);              // bit 1 is type = on/off
  writeReg1Byte(I2CEXTPL_SLOT_FUNC, slotFunc);
  writeReg2Byte(I2CEXTPL_SLOT_DURATION, duration);
  return(writeReg1Byte(I2CEXTPL_SLOT_NR, slotNr));
}
//-------------------------------------------------------------------------------------
bool I2CEXTPL::setOutputPulse(uint8_t slotNr, uint16_t hTime, uint16_t lTime, uint16_t duration)
{
  byte slotFunc = 0;
  slotFunc |= (1<<SLT_PULSE_BIT);               // bit 2 is type = Blink
  writeReg1Byte(I2CEXTPL_SLOT_FUNC, slotFunc);
  writeReg2Byte(I2CEXTPL_SLOT_LOWTIME, hTime);
  writeReg2Byte(I2CEXTPL_SLOT_HIGHTIME, lTime);
  writeReg2Byte(I2CEXTPL_SLOT_DURATION, duration);
  return(writeReg1Byte(I2CEXTPL_SLOT_NR, slotNr));
}

//-------------------------------------------------------------------------------------
bool I2CEXTPL::setDebounceTime(uint8_t microsecs)
{
  return (writeReg1Byte(I2CEXTPL_DEBOUNCETIME, microsecs));
}
//-------------------------------------------------------------------------------------
bool I2CEXTPL::setMidPressTime(uint16_t millisecs)
{
  return (writeReg2Byte(I2CEXTPL_MIDPRESSTIME, millisecs));
}
//-------------------------------------------------------------------------------------
bool I2CEXTPL::setLongPressTime(uint16_t millisecs)
{
  return (writeReg2Byte(I2CEXTPL_LONGPRESSTIME, millisecs));
}
//-------------------------------------------------------------------------------------
bool I2CEXTPL::writeCommand(byte command)
{
  return (writeReg1Byte(I2CEXTPL_COMMAND, command));
}

//-------------------------------------------------------------------------------------
//-------------------------- GETTERS --------------------------------------------------
//-------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------
uint8_t I2CEXTPL::getSysStatus()
{
  while ((millis() - _readTimer) < _READDELAY) 
  {
    delay(1);
  }
  _readTimer = millis();
  _SYSstatus = (uint8_t)readReg1Byte(I2CEXTPL_SYSSTATUS);
  return (_SYSstatus);
}

//-------------------------------------------------------------------------------------
uint8_t I2CEXTPL::getSlotModes()
{
  while ((millis() - _readTimer) < _READDELAY) 
  {
    delay(1);
  }
  _readTimer = millis();
  uint8_t tmpStatus = (uint8_t)readReg1Byte(I2CEXTPL_SLOT_MODES);
  _SLOTmodes = (uint8_t)tmpStatus;
  return (_SLOTmodes);
}

//-------------------------------------------------------------------------------------
uint8_t I2CEXTPL::getSlotStatus(uint8_t slotNr)
{
  while ((millis() - _readTimer) < _READDELAY) 
  {
    delay(1);
  }
  _readTimer = millis();
  uint8_t tmpStatus = (uint8_t)readReg1Byte(I2CEXTPL_SLOTSTATUS + slotNr);
  _SLOTstatus[slotNr] = (uint8_t)tmpStatus;
  return (_SLOTstatus[slotNr]);
}

//-------------------------------------------------------------------------------------
int8_t I2CEXTPL::getWhoAmI()
{
  return (readReg1Byte(I2CEXTPL_ADDRESS));
}

//-------------------------------------------------------------------------------------
uint8_t I2CEXTPL::getDebounceTime()
{
  return (readReg1Byte(I2CEXTPL_DEBOUNCETIME));
}
//-------------------------------------------------------------------------------------
uint16_t I2CEXTPL::getMidPressTime()
{
  return (readReg2Byte(I2CEXTPL_MIDPRESSTIME));
}
//-------------------------------------------------------------------------------------
uint16_t I2CEXTPL::getLongPressTime()
{
  return (readReg2Byte(I2CEXTPL_LONGPRESSTIME));
}
//-------------------------------------------------------------------------------------
uint8_t I2CEXTPL::getModeSettings()
{
  return (readReg1Byte(I2CEXTPL_MODESETTINGS));
}
//-------------------------------------------------------------------------------------
bool I2CEXTPL::getModeSettings(uint8_t testBit)
{
  return ( readReg1Byte(I2CEXTPL_MODESETTINGS) & (1<<testBit) );
}
//-------------------------------------------------------------------------------------
uint8_t I2CEXTPL::getMajorRelease()
{
  return (readReg1Byte(I2CEXTPL_MAJORRELEASE));
}
//-------------------------------------------------------------------------------------
uint8_t I2CEXTPL::getMinorRelease()
{
  return (readReg1Byte(I2CEXTPL_MINORRELEASE));
}


//-------------------------------------------------------------------------------------
//-------------------------- READ FROM REGISTERS --------------------------------------
//-------------------------------------------------------------------------------------

// Reads a uint8_t from a register @addr
//-------------------------------------------------------------------------------------
uint8_t I2CEXTPL::readReg1Byte(uint8_t addr)
{  
  while ((millis() - _readTimer) < _READDELAY) {
    delay(1);
  }
  _readTimer = millis();

  _I2Cbus->beginTransmission((uint8_t)_I2Caddress);
  _I2Cbus->write(addr);
  if (_I2Cbus->endTransmission() != 0) {
    return (0); // Slave did not ack
  }

  _I2Cbus->requestFrom((uint8_t)_I2Caddress, (uint8_t) 1);
  if (_I2Cbus->available()) {
    return (_I2Cbus->read());
  }

  return (0); // Slave did not respond
}

// Reads an int16_t from a register @addr
//-------------------------------------------------------------------------------------
int16_t I2CEXTPL::readReg2Byte(uint8_t addr)
{
  while ((millis() - _readTimer) < _READDELAY) {
    delay(1);
  }
  _readTimer = millis();

  _I2Cbus->beginTransmission((uint8_t)_I2Caddress);
  _I2Cbus->write(addr);
  if (_I2Cbus->endTransmission() != 0) {
    return (0); // Slave did not ack
  }

  _I2Cbus->requestFrom((uint8_t)_I2Caddress, (uint8_t) 2);
  if (_I2Cbus->available()) {
    uint8_t LSB = _I2Cbus->read();
    uint8_t MSB = _I2Cbus->read();
    return ((int16_t)MSB << 8 | LSB);
  }

  return (0); // Slave did not respond
}

// Reads an int32_t from a register @addr
//-------------------------------------------------------------------------------------
int32_t I2CEXTPL::readReg4Byte(uint8_t addr)
{
  while ((millis() - _readTimer) < _READDELAY) {
    delay(1);
  }
  _readTimer = millis();

  _I2Cbus->beginTransmission((uint8_t)_I2Caddress);
  _I2Cbus->write(addr);
  if (_I2Cbus->endTransmission() != 0) {
    return (0); // Slave did not ack
  }

  _I2Cbus->requestFrom((uint8_t)_I2Caddress, (uint8_t) 4);
  delay(10);
  if (_I2Cbus->available()) {
    uint8_t LSB   = _I2Cbus->read();
    uint8_t mLSB  = _I2Cbus->read();
    uint8_t mMSB  = _I2Cbus->read();
    uint8_t MSB   = _I2Cbus->read();
    uint32_t comb = (uint32_t)MSB << 24 | (uint32_t)mMSB << 16 | (uint32_t)mLSB << 8 | LSB;
    return (comb);
  }

  return (0); // Slave did not respond
}

//-------------------------------------------------------------------------------------
//-------------------------- WRITE TO REGISTERS ---------------------------------------
//-------------------------------------------------------------------------------------

// Write a 1 byte value to a register
//-------------------------------------------------------------------------------------
bool I2CEXTPL::writeReg1Byte(uint8_t addr, uint8_t val)
{
  while ((millis() - _readTimer) < _WRITEDELAY) {
    delay(1);
  }
  _readTimer = millis();

  _I2Cbus->beginTransmission((uint8_t)_I2Caddress);
  _I2Cbus->write(addr);
  _I2Cbus->write(val);
  if (_I2Cbus->endTransmission() != 0) {
    return (false); // Slave did not ack
  }

  return (true);
}

// Write a 2 byte value to a register
//-------------------------------------------------------------------------------------
bool I2CEXTPL::writeReg2Byte(uint8_t addr, int16_t val)
{
  while ((millis() - _readTimer) < _WRITEDELAY) {
    delay(1);
  }
  _readTimer = millis();

  _I2Cbus->beginTransmission((uint8_t)_I2Caddress);
  _I2Cbus->write(addr);
  _I2Cbus->write(val & 0xFF); // LSB
  _I2Cbus->write(val >> 8);   // MSB
  if (_I2Cbus->endTransmission() != 0) {
    return (false); // Slave did not ack
  }

  return (true);
}


// Write a 3 byte value to a register
//-------------------------------------------------------------------------------------
bool I2CEXTPL::writeReg3Byte(uint8_t addr, int32_t val)
{
  while ((millis() - _readTimer) < _WRITEDELAY) {
    delay(1);
  }
  _readTimer = millis();

  _I2Cbus->beginTransmission((uint8_t)_I2Caddress);
  _I2Cbus->write(addr);
  _I2Cbus->write(val &0xFF);      // LSB
  _I2Cbus->write(val >> 8);       // mLSB
  _I2Cbus->write(val >> 16);      // mMSB
  //_I2Cbus->write(val >> 24);    // MSB
  if (_I2Cbus->endTransmission() != 0) {
    return (false); // Slave did not ack
  }

  return (true);
}

// Write a 4 byte value to a register
//-------------------------------------------------------------------------------------
bool I2CEXTPL::writeReg4Byte(uint8_t addr, int32_t val)
{
  while ((millis() - _readTimer) < _WRITEDELAY) {
    delay(1);
  }
  _readTimer = millis();

  _I2Cbus->beginTransmission((uint8_t)_I2Caddress);
  _I2Cbus->write(addr);
  _I2Cbus->write(val & 0xFF); // LSB
  _I2Cbus->write(val >> 8);   // mLSB
  _I2Cbus->write(val >> 16);  // mMSB
  _I2Cbus->write(val >> 24);  // MSB
  if (_I2Cbus->endTransmission() != 0) {
    return (false); // Slave did not ack
  }

  return (true);
}

//-------------------------------------------------------------------------------------
//-------------------------- HELPERS --------------------------------------------------
//-------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------
bool I2CEXTPL::isConnected()
{
  _I2Cbus->beginTransmission((uint8_t)_I2Caddress);
  if (_I2Cbus->endTransmission() != 0)
    return (false); // I2C Slave did not ACK
  return (true);
} // isConnected()

//-------------------------------------------------------------------------------------
bool I2CEXTPL::isSlotPressed(uint8_t slotNr) 
{
  if (_SLOTstatus[slotNr] & (1 << SLT_PRESSED_BIT)) {
    _SLOTstatus[slotNr] &= ~(1 << SLT_PRESSED_BIT);
    return true;
  }
  return false;
}

//-------------------------------------------------------------------------------------
bool I2CEXTPL::isSlotQuickReleased(uint8_t slotNr)
{
  if (_SLOTstatus[slotNr] & (1 << SLT_QUICKRELEASE_BIT)) {
    _SLOTstatus[slotNr] &= ~(1 << SLT_QUICKRELEASE_BIT);
    return true;
  }
  return false;
}

//-------------------------------------------------------------------------------------
bool I2CEXTPL::isSlotMidReleased(uint8_t slotNr)
{
  if (_SLOTstatus[slotNr] & (1 << SLT_MIDRELEASE_BIT)) {
    _SLOTstatus[slotNr] &= ~(1 << SLT_MIDRELEASE_BIT);
    return true;
  }
  return false;
}

//-------------------------------------------------------------------------------------
bool I2CEXTPL::isSlotLongReleased(uint8_t slotNr)
{
  if (_SLOTstatus[slotNr] & (1 << SLT_LONGRELEASE_BIT)) {
    _SLOTstatus[slotNr] &= ~(1 << SLT_LONGRELEASE_BIT);
    return true;
  }
  return false;
}

//===========================================================================================
//assumes little endian
void I2CEXTPL::printRegister(HardwareSerial *serOut, size_t const size, void const * const ptr)
{
  //assumes little endian
  unsigned char *b = (unsigned char*) ptr;
  unsigned char byte;
  int i, j;
  serOut->print("(lib) [");
  for (i=size-1;i>=0;i--)
  {
    if (i!=(size-1)) serOut->print(" ");
    for (j=7;j>=0;j--)
    {
      byte = (b[i] >> j) & 1;
      serOut->print(byte);
    }
  }
  serOut->print("]");

} // printRegister()



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
