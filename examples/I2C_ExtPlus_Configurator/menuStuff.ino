
/*
***************************************************************************  
**
**  Program     : menuStuff (part of I2C_Configurator)
**    
**  Copyright (c) 2020 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/

//===========================================================================================
bool valueInRange(int32_t val, int32_t minVal, int32_t maxVal)
{
  if (val < minVal) return false;
  if (val > maxVal) return false;
  return true;
} // valueInRange()

//===========================================================================================
int32_t readNumber(char *prompt, int32_t minVal, int32_t maxVal)
{
  int32_t inVal, outVal;
  String in;
  do {
    Serial.println();
    Serial.print(prompt);
    Serial.print(F("("));
    Serial.print(minVal);
    Serial.print(F(" t/m "));
    Serial.print(maxVal);
    Serial.print(F(") : "));
    while(Serial.available()) Serial.read();
    Serial.setTimeout(30000); // dertig seconden
    in = Serial.readStringUntil('\n'); 
    inVal = String(in).toInt();
  } while (!valueInRange(inVal, minVal, maxVal));

  Serial.println(inVal);

  return inVal;

} // readNumber()



//===========================================================================================
void handleKeyInput()
{
  char    inChar;
  int32_t val;
  uint8_t command = 0;
  uint8_t modeReg;
  while (Serial.available() > 0) 
  {
    delay(1);
    inChar = (char)Serial.read();
    if (inChar == '\r') continue;

    switch(inChar) 
    {
      case 'A':
            I2C_newAddress = readNumber("Change I2C address (dec)", 1, 127);
            if (!ExtenderBoard.setI2Caddress(I2C_newAddress & 0x7F)) 
            {
              Serial.println(F("Error setting new I2C address .."));
              break;
            } 
            Serial.print(F("New I2C address set to [0x"));
            if (I2C_newAddress < 0x0F) Serial.print("0");
            Serial.print(I2C_newAddress, HEX);
            Serial.println(F("]"));
            Serial.println(F("--> Save registers to EEPROM (S)"));
            break;
      case '-':
            doAnimate = false;
            break;

      case '+':
            doAnimate = true;
            break;
      case 'j':
      case 'J':
            val = readNumber("Debounce time in micro seconds", 1, 255);
            if (!ExtenderBoard.setDebounceTime(val & 0xFF)) 
            {
              Serial.println(F("Error setting DebounceTime.."));
            }
            break;
      case 'k':
      case 'K':
            val = readNumber("Mid Press Time (milli sec.)", 100, 5000);
            if (!ExtenderBoard.setMidPressTime(val)) 
            {
              Serial.println(F("Error setting Mid Press Time .."));
            }
            break;
      case 'l':
      case 'L':
            val = readNumber("Long Press Time (milli sec.)", 300, 10000);
            if (!ExtenderBoard.setLongPressTime(val)) 
            {
              Serial.println(F("Error setting Long Press Time .."));
            }
            break;

      case 's':
      case 'S':
            if (ExtenderBoard.writeCommand(_BV(CMD_WRITECONF))) 
            {
              Serial.println(F("Registers saved to EEPROM"));
            } 
            else 
            {
              Serial.println(F("Error saving to EEPROM"));
            }
            delay(250); // <-- give Slave some time to save to EEPROM
            if (ExtenderBoard.writeCommand(_BV(CMD_REBOOT))) 
            {
              Serial.println(F("Slave is reBooting .."));
              inConfigureMode = false;
            } 
            else 
            {
              Serial.println(F("Slave is not reBooting ??"));
            }
            break;
      case 'r':
      case 'R':
            if (ExtenderBoard.writeCommand(_BV(CMD_READCONF))) 
            {
              Serial.println(F("registers read from EEPROM .."));
            } 
            else 
            {
              Serial.println(F("Error reading from EEPROM"));
            }
            break;
      case 'X':
            if (ExtenderBoard.writeCommand(_BV(CMD_REBOOT))) 
            {
              Serial.println(F("Slave is reBooting .."));
              inConfigureMode = false;
            } 
            else 
            {
              Serial.println(F("Slave is not reBooting ??"));
            }
            break;
      case 'z':
      case 'Z':
            Serial.println(F("Z (exit config mode)"));
            inConfigureMode = false;
            break;

      default:
            Serial.println(F("\n===================================="));
            Serial.println(F(  "=========== Main menu =============="));
            Serial.println(F(  "===================================="));
            Serial.print(F("\n==== Firmware version I2C_ExtPlus ========[v"));
            Serial.print(majorRelease);
            Serial.print(F("."));
            Serial.print(minorRelease);
            Serial.println(F("]============================"));
            Serial.print(F("*A.  Change I2C address .............. (is now [0x"));
            Serial.print(I2C_Address, HEX);
            Serial.print(F(", dec"));
            Serial.print(I2C_Address);
            Serial.print(F("])"));
            if (I2C_Address != ExtenderBoard.getWhoAmI()) 
            {
              Serial.print(F(" *! ["));
              if (I2C_newAddress < 0x0F) Serial.print("0");
              Serial.print(I2C_newAddress, HEX);
              Serial.println(F("]"));
            } 
            else  Serial.println();
            Serial.print(F(" J.  Set debounceTime in microSeconds  (is now ["));
            Serial.print(ExtenderBoard.getDebounceTime());
            Serial.println(F("])"));
            Serial.print(F(" K.  Set Mid Press Time in milliSec.   (is now ["));
            Serial.print(ExtenderBoard.getMidPressTime());
            Serial.println(F("])"));
            Serial.print(F(" L.  Set Long Press Time in milliSec.  (is now ["));
            Serial.print(ExtenderBoard.getLongPressTime());
            Serial.println(F("])"));

            Serial.println(F(" "));
            Serial.println(F(" S.  Save registers to EEPROM"));
            Serial.println(F(" R.  Read registers from EEPROM"));
            Serial.println(F(" "));
            Serial.println(F("*X.  Reboot Slave"));
            Serial.println(F(" Z.  Exit"));
            Serial.println(F(" "));
            
    } // switch()
    while (Serial.available() > 0) {
      delay(0);
      (char)Serial.read();
    }
  }

}  // handleKeyInput()


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
