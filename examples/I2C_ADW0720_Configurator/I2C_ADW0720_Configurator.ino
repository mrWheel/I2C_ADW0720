/*
***************************************************************************  
**
**  Program     : I2C_ADW0720_Configurator
*/
#define _FW_VERSION  "v1.5 (24-08-2020)"
/*
**  Description : With this program you can configure 
**                I2C extender boards
**    
**  Copyright (c) 2020 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/


#define  I2C_DEFAULT_ADDRESS  0x18    // the 7-bit address 
//#define _SDA                  4
//#define _SCL                  5

#define SETBIT(regByte, bit)       (regByte) |=  (1 << (bit))
#define CLEARBIT(regByte, bit)     (regByte) &= ~(1 << (bit))
#define BIT_IS_HIGH(regByte, bit)  ((regByte) & (1<<(bit)))
#define BIT_IS_LOW(regByte, bit)   (!(BIT_IS_HIGH(regByte, bit)))

#define SLOT0                   0
#define SLOT1                   1
#define SLOT2                   2
#define SLOT3                   3
#define SLOT4                   4
#define SLOT5                   5
#define SLOT6                   6
#define SLOT7                   7

#include <I2C_ADW0720.h>

I2CADW0720    ExtenderBoard; //-- Create instance of this object

static byte   I2C_Address = 0x00, I2C_newAddress;

byte          whoAmI;
byte          majorRelease, minorRelease;
uint8_t       sysStatus, slotStatus, slotModes;

uint32_t      builtinLedTimer;

bool          inConfigureMode   = false;


//===========================================================================================
byte walkTheSlots(bool dirUp)
{
  byte slotModes = ExtenderBoard.getSlotModes();

  Serial.println("Walk the lights ...");
  if (dirUp)
  {
    for(int8_t slt=0; slt<8; slt++)
    { 
      if (BIT_IS_LOW(slotModes, slt) )
      {
        ExtenderBoard.setOutputPulse(slt, 500,2000,10000);
      }
    }
    return;
  }
  if (!dirUp)
  {
    for(int8_t slt=7; slt>=0; slt--)
    { 
      if (BIT_IS_LOW(slotModes, slt) )
      {
        ExtenderBoard.setOutputPulse(slt, 200,1000,10000);
      }
    }
    return;
  }

} // walkTheSlots()


//===========================================================================================
byte findSlaveAddress(byte startAddress)
{
  byte  error;
  bool  slaveFound = false;

  if (startAddress == 0xFF || startAddress == 0x00) startAddress = 1;
  for (byte address = startAddress; address <= 127; address++) 
  {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error) 
    {
      //Serial.print(F("Error["));
      //Serial.print(error);
      //Serial.println(F("]"));
      slaveFound = false;
    } 
    else 
    {
      slaveFound = true;
      Serial.print(F("\nFound one!\nDo you want to configure I2C ADW0720 board at @[0x"));
      if (address < 0x0F) Serial.print("0");
      Serial.print(address , HEX);
      Serial.print(F("] (y/N) : "));
      Serial.println();
      String answerS = "";
      char answerC = '-';
      bool stayInWhile = true;
      while ((answerC != 'y' && answerC != 'Y') && stayInWhile) 
      {
        Serial.setTimeout(10000);
        answerS = Serial.readStringUntil('\n');
        answerS.trim();
        answerC = answerS[answerS.length()-1];
        if (answerC == 'y' || answerC == 'Y') 
        {
          Serial.println(F("\nNow in configuration mode .."));
          inConfigureMode = true;
          return address;
        }
        Serial.println(F("trying next address .."));
        inConfigureMode = false;
        stayInWhile     = false;
        slaveFound      = false;
      }
    }
  }
  if (slaveFound) 
  {
    Serial.println(F("\nstart reScan ..\n"));
  } 
  else 
  {
    Serial.println(F("\n\nNo ADW0720 board found!! -> try again in 5 seconds .."));
    delay(5000);
  }
  return 0xFF;

} // findSlaveAddress()


//===========================================================================================
void handleInputSlots(uint8_t slotNr)
{
  slotStatus = ExtenderBoard.getSlotStatus(slotNr);
  if (slotStatus == 0) return;
  
  Serial.print(F("sysStatus: "));
  ExtenderBoard.printRegister(&Serial, 1, &sysStatus);
  sysStatus = ExtenderBoard.getSysStatus();
  Serial.print(F(", slotStatus: "));
  ExtenderBoard.printRegister(&Serial, 1, &slotStatus);
  Serial.println();
  
  Serial.print(F("-------> Slot["));
  Serial.print(slotNr);
  if (ExtenderBoard.isSlotPressed(slotNr) ) {
    Serial.print(F("] Pressed"));
  }
  if (ExtenderBoard.isSlotQuickReleased(slotNr) ) {
    Serial.print(F("] Quick Release"));
  }
  if (ExtenderBoard.isSlotMidReleased(slotNr) ) {
    Serial.print(F("] Mid Release"));
  }
  if (ExtenderBoard.isSlotLongReleased(slotNr) ) {
    Serial.print(F("] Long Release"));
  }
  Serial.println();

} // handleInputSlots();


//===========================================================================================
//===========================================================================================
void setup()
{
  Serial.begin(115200);
  Serial.println(F("\r\nStart I2C-ADW0720 Extender Configurator ....\r\n"));
  Serial.print(F("Setup Wire .."));
//Wire.begin(_SDA, _SCL); // join i2c bus (address optional for master)
  Wire.begin();
  delay(1000);
//Wire.setClock(400000L);
  Serial.println(F(".. done"));

  Serial.println(F("setup() done .. \n"));

} // setup()


//===========================================================================================
void loop()
{

  if (!inConfigureMode) 
  {
    I2C_Address = findSlaveAddress(I2C_Address);
    if (I2C_Address != 0xFF && I2C_Address != 0x00) 
    {
      Serial.println(F("\nConnecting to  I2C-ADW0720 board .."));
      if (ExtenderBoard.begin(Wire, I2C_Address)) 
      {
        if (!ExtenderBoard.writeCommand(_BV(CMD_READCONF))) 
        {
          Serial.println(F("Error reading from EEPROM"));
        }
        delay(50);
        majorRelease = ExtenderBoard.getMajorRelease();
        delay(50);
        minorRelease = ExtenderBoard.getMinorRelease();
        Serial.print(F(". connected with slave @[0x"));
        Serial.print(I2C_Address, HEX);
        Serial.print(F("] Release[v"));
        Serial.print(majorRelease);
        Serial.print(F("."));
        Serial.print(minorRelease);
        Serial.println(F("]"));
        delay(50);
        I2C_newAddress = ExtenderBoard.getWhoAmI();
      } 
      else 
      {
        Serial.println(F(".. Error connecting to I2C slave .."));
      }
    } 
    else 
    {
      return;
    }
  }

  if (inConfigureMode) handleKeyInput();

  sysStatus = ExtenderBoard.getSysStatus();
  if (sysStatus == 0) return;
  for(uint8_t slt=0; slt<8; slt++)
  { 
    if (BIT_IS_HIGH(slotModes, slt) )
    {
      handleInputSlots(slt);
    }
    else
    {
      //animateLeds();
    }
  }

} // loop()

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
