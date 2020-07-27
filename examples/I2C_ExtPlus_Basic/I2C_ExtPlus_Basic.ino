/*
***************************************************************************
**
**  Program     : I2C_ExtPlus_Basic
*/
#define _FW_VERSION  "v1.1 (27-07-2020)"
/*
**  Description : Demo "howto" use I2C_ExtPlus board
**
**  Copyright (c) 2020 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.
***************************************************************************
*/

#define SETBIT(regByte, bit)     (regByte) |=  (1 << (bit))
#define CLEARBIT(regByte, bit)   (regByte) &= ~(1 << (bit))
#define ISBITHIGH(regByte, bit)  ((regByte) & (1<<(bit)))
#define ISBITLOW(regByte, bit)   (!((regByte) & (1<<(bit))))


#define I2C_DEFAULT_ADDRESS  0x18    // the 7-bit address 
//#define _SDA                  4
//#define _SCL                  5

#define LED1                  0
#define LED2                  1
#define LED3                  2
#define LED4                  3
#define BUTTON1               4
#define BUTTON2               5
#define BUTTON3               6
#define BUTTON4               7

#include <I2C_ExtPlus.h>


I2CEXTPL ExtPlusBoard; // Create instance of the I2CEXTPL object

byte          whoAmI;
byte          majorRelease, minorRelease;
uint32_t      builtinLedTimer;
int8_t        inMode = 0;


//===========================================================================================
void handleInput()
{
  byte statusReg, sysStatus, slotModes;
  static bool sLed1, sLed2, sLed3, sLed4;
  
  sysStatus = ExtPlusBoard.getSysStatus();
  if (sysStatus == 0) return;

  Serial.print("sysStatus: ");
  ExtPlusBoard.printRegister(&Serial, sizeof(sysStatus), &sysStatus);
  //slotModes = ExtPlusBoard.getSlotModes();
  //ExtPlusBoard.printRegister(&Serial, sizeof(slotModes), &slotModes);
  Serial.println();
  
  if ( ISBITHIGH(sysStatus, BUTTON1) )  
  {
    statusReg = ExtPlusBoard.getSlotStatus(BUTTON1);
    Serial.print("slotStatus[BUTTON1]: ");
    ExtPlusBoard.printRegister(&Serial, 1, &statusReg);
    Serial.println();
    
    if (ExtPlusBoard.isSlotPressed(BUTTON2) ) 
    {
      Serial.println(F("(BUTTON1)-------> Button Pressed"));
      ExtPlusBoard.setOutputToggle(LED1, HIGH, 200);
    }
    if (ExtPlusBoard.isSlotQuickReleased(BUTTON1) ) 
    {
      Serial.println(F("(BUTTON1)-------> Quick Release "));
      if (sLed1) 
      {
        Serial.println("\t setOutputToggle(LED1, LOW, 0);");
        ExtPlusBoard.setOutputToggle(LED1, LOW, 0);
        sLed1 = false;
      }
      else
      {
        Serial.println("\t setOutputToggle(LED1, HIGH, 5000);");
        ExtPlusBoard.setOutputToggle(LED1, HIGH, 5000);
        sLed1 = true;
      }
    }
    if (ExtPlusBoard.isSlotMidReleased(BUTTON1) ) 
    {
      Serial.println(F("(BUTTON1)-------> Mid Release   "));
      Serial.println("\t setOutputPulse(LED1, 50, 50, 0);");
      ExtPlusBoard.setOutputPulse(LED1, 50, 50, 0);
    }
    if (ExtPlusBoard.isSlotLongReleased(BUTTON1) ) 
    {
      Serial.println(F("(BUTTON1)-------> Long Release  "));
      Serial.println("\t setOutputPulse(LED1, 100, 100, 0);");
      ExtPlusBoard.setOutputPulse(LED1, 100, 100, 0);
    }
  } // bitRead(slot0)
  
  if ( (sysStatus & _BV(BUTTON2)) != 0 )  
  {
    statusReg = ExtPlusBoard.getSlotStatus(BUTTON2);
    Serial.print("slotStatus[BUTTON2]: ");
    ExtPlusBoard.printRegister(&Serial, 1, &statusReg);
    Serial.println();

    if (ExtPlusBoard.isSlotPressed(BUTTON2) ) 
    {
      Serial.println(F("(BUTTON2)-------> Button Pressed"));
      ExtPlusBoard.setOutputToggle(LED2, HIGH, 200);
    }
    if (ExtPlusBoard.isSlotQuickReleased(BUTTON2) ) 
    {
      Serial.println(F("(BUTTON2)-------> Quick Release "));
      Serial.println("\t setOutputToggle(LED2, HIGH, 5000);");
      ExtPlusBoard.setOutputToggle(LED2, HIGH, 5000);
    }
    if (ExtPlusBoard.isSlotMidReleased(BUTTON2) ) 
    {
      Serial.println(F("(BUTTON2)-------> Mid Release   "));
      Serial.println("\t setOutputPulse(LED2, 100, 150, 0);");
      ExtPlusBoard.setOutputPulse(LED2, 100, 150, 0);
    }
    if (ExtPlusBoard.isSlotLongReleased(BUTTON2) ) 
    {
      Serial.println(F("(BUTTON2)-------> Long Release  "));
      Serial.println("\t setOutputPulse(LED2, 300, 500, 0);");
      ExtPlusBoard.setOutputPulse(LED2, 300, 500, 0);
    }
  } // bitRead(BUTTON2)
  
  if ( (sysStatus & _BV(BUTTON3)) != 0 )  
  {
    statusReg = ExtPlusBoard.getSlotStatus(BUTTON3);
    Serial.print("Status[BUTTON3]: ");
    ExtPlusBoard.printRegister(&Serial, 1, &statusReg);
    Serial.println();

    if (ExtPlusBoard.isSlotPressed(BUTTON3) ) 
    {
      Serial.println(F("(BUTTON3)-------> Button Pressed"));
      ExtPlusBoard.setOutputToggle(LED3, HIGH, 200);
    }
    if (ExtPlusBoard.isSlotQuickReleased(BUTTON3) ) 
    {
      Serial.println(F("(BUTTON3)-------> Quick Release "));
      Serial.println("\t setOutputToggle(LED3, HIGH, 5000);");
      ExtPlusBoard.setOutputToggle(LED3, HIGH, 5000);
    }
    if (ExtPlusBoard.isSlotMidReleased(BUTTON3) ) 
    {
      Serial.println(F("(BUTTON3)-------> Mid Release   "));
      Serial.println("\t setOutputPulse(LED3, 700, 900, 0);");
      ExtPlusBoard.setOutputPulse(LED3, 700, 900, 0);
    }
    if (ExtPlusBoard.isSlotLongReleased(BUTTON3) ) 
    {
      Serial.println(F("(BUTTON3)-------> Long Release  "));
      Serial.println("\t setOutputPulse(LED3, 1000, 1200, 0);");
      ExtPlusBoard.setOutputPulse(LED3, 1000, 1200, 0);
    }
  } // bitRead(BUTTON3)
  
  if ( (sysStatus & _BV(BUTTON4)) != 0 )  
  {
    statusReg = ExtPlusBoard.getSlotStatus(BUTTON4);
    Serial.print("Status[BUTTON4]: ");
    ExtPlusBoard.printRegister(&Serial, 1, &statusReg);
    Serial.println();
    
    if (ExtPlusBoard.isSlotPressed(BUTTON4) ) 
    {
      Serial.println(F("(BUTTON4)-------> Button Pressed"));
      ExtPlusBoard.setOutputToggle(LED4, HIGH, 200);
    }
    if (ExtPlusBoard.isSlotQuickReleased(BUTTON4) ) 
    {
      Serial.println(F("(BUTTON4)-------> Quick Release "));
      Serial.println("\t setOutputToggle(LED4, HIGH, 5000);");
      ExtPlusBoard.setOutputToggle(LED4, HIGH, 5000);
    }
    if (ExtPlusBoard.isSlotMidReleased(BUTTON4) ) 
    {
      Serial.println(F("(BUTTON4)-------> Mid Release   "));
      Serial.println("\t setOutputPulse(LED4, 700, 1000, 0);");
      ExtPlusBoard.setOutputPulse(LED4, 700, 1000, 0);
    }
    if (ExtPlusBoard.isSlotLongReleased(BUTTON4) ) 
    {
      Serial.println(F("(BUTTON4)-------> Long Release  "));
      Serial.println("\t setOutputPulse(LED4, 1000, 1200, 0);");
      ExtPlusBoard.setOutputPulse(LED4, 1000, 1200, 0);
    }
  } // bitRead(BUTTON4)

} // handleInput();


//===========================================================================================
void setup()
{
  Serial.begin(115200);
  while(!Serial) { /* wait a bit */ }
  
  Serial.println(F("\r\nStart I2C_EXTPL_Test ....\r\n"));
  Serial.print(F("Setup Wire .."));
//Wire.begin(_SDA, _SCL); // join i2c bus (address optional for master)
  Wire.begin();
//Wire.setClock(400000L);
  Serial.println(F(".. done"));

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  if (ExtPlusBoard.begin(Wire, I2C_DEFAULT_ADDRESS)) 
  {
    majorRelease = ExtPlusBoard.getMajorRelease();
    minorRelease = ExtPlusBoard.getMinorRelease();
    Serial.print(F(". connected with slave @[0x"));
    Serial.print(I2C_DEFAULT_ADDRESS, HEX);
    Serial.print(F("] Release[v"));
    Serial.print(majorRelease);
    Serial.print(F("."));
    Serial.print(minorRelease);
    Serial.println(F("]"));
  } 
  else 
  {
    Serial.println(F(".. Error connecting to I2C slave .."));
    delay(1000);
    return;
  }

  Serial.print("debounceTime[");  Serial.print(ExtPlusBoard.getDebounceTime());  Serial.println("]");
  Serial.print("MidPressTime[");  Serial.print(ExtPlusBoard.getMidPressTime());  Serial.println("]");
  Serial.print("LongPressTime["); Serial.print(ExtPlusBoard.getLongPressTime()); Serial.println("]");

  byte slotModes = ExtPlusBoard.getSlotModes();
  ExtPlusBoard.printRegister(&Serial, sizeof(slotModes), &slotModes);
  Serial.println();
  for (int i=0; i<8; i++)
  {
    Serial.print("Bit ["); Serial.print(i); Serial.print("] is ");
    if (ISBITHIGH(slotModes, i))  
          Serial.println("HIGH");
    else  Serial.println("LOW");
  }
  
  //ExtPlusBoard.setModeInput(BUTTON1);
  //ExtPlusBoard.setModeInput(BUTTON2);
  //ExtPlusBoard.setModeInput(BUTTON3);
  //ExtPlusBoard.setModeInput(BUTTON4);

  //ExtPlusBoard.setModeOutput(LED1);
  Serial.println("> setOutputToggle(LED1, HIGH, 500)");
  ExtPlusBoard.setOutputToggle(LED1, HIGH, 500);
  delay(100);
  //ExtPlusBoard.setModeOutput(LED2);
  Serial.println("> setOutputToggle(LED2, HIGH, 500)");
  ExtPlusBoard.setOutputToggle(LED2, HIGH, 500);
  delay(100);
  //ExtPlusBoard.setModeOutput(LED3);
  Serial.println("> setOutputToggle(LED3, HIGH, 500)");
  ExtPlusBoard.setOutputToggle(LED3, HIGH, 500);
  delay(100);
  //ExtPlusBoard.setModeOutput(LED4);
  Serial.println("> setOutputToggle(LED4, HIGH, 500)");
  ExtPlusBoard.setOutputToggle(LED4, HIGH, 500);

  Serial.println(F("setup() done .. \n"));

} // setup()


//===========================================================================================
void loop()
{
  handleInput();

  if ((millis() - builtinLedTimer) > 2000) 
  {
    builtinLedTimer = millis();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
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
****************************************************************************
*/
