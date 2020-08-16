/*
**    Program : EepromStuff (part of I2C_ADW0720_Tiny841_Slave)
**
**    Copyright (c) 2020 Willem Aandewiel
**
**    TERMS OF USE: MIT License. See bottom of file.
***************************************************************************
*/


//--------------------------------------------------------------------------
static void readConfig()
{
  registerLayout registersSaved;

  eeprom_read_block(&registersSaved, 0, sizeof(registersSaved));
  //--- the registerStack will not change in the same Major Version --
  if ( registersSaved.majorRelease == _MAJOR_RELEASE ) 
  {
    registerStack = registersSaved;
    registerStack.minorRelease = _MINOR_RELEASE;
    digitalWrite(LED_BUILTIN, HIGH);

  //--- the Major version has changed and there is no way of -----
  //--- knowing the registerStack has not changed, so rebuild ----
  } else {  
    registerStack.address       = _I2C_DEFAULT_ADDRESS;
    registerStack.majorRelease  = _MAJOR_RELEASE;
    registerStack.minorRelease  = _MINOR_RELEASE;
    registerStack.slotModes     =  0xFF;
    registerStack.debounceTime  =    10;
    registerStack.midPressTime  =   500;
    registerStack.longPressTime =  1500;
    registerStack.modeSettings  =  0x0F;

    digitalWrite(LED_BUILTIN, LOW);
    //*slot[2].port |= _BV(slot[2].bit); // cannot use this until config is read
    writeConfig();
  }

  normalizeSettings();

} // readConfig()

//--------------------------------------------------------------------------
static void writeConfig()
{

  eeprom_update_block(&registerStack, 0, sizeof(registerStack));
//eeprom_write_block(&registerStack, 0, sizeof(registerStack));

  digitalWrite(LED_BUILTIN, LOW);

} // writeConfig()

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
****************************************************************************
*/
