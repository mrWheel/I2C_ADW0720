/*
***************************************************************************
**
**  Program     : I2C_ADW0720_Basic
*/
#define _FW_VERSION  "v1.3 (31-07-2020)"
/*
**  Description : Demo "howto" use I2C_ADW0720 board
**
**  Copyright (c) 2020 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.
***************************************************************************
*/

//#define VERBOSE   // uncomment for max verbosity

#define SETBIT(regByte, bit)       ((regByte) |=  (1 << (bit)))
#define CLEARBIT(regByte, bit)     ((regByte) &= ~(1 << (bit)))
#define BIT_IS_HIGH(regByte, bit)  ((regByte) &   (1<<(bit)))
#define BIT_IS_LOW(regByte, bit)   (!((regByte) & (1<<(bit))))


#define I2C_DEFAULT_ADDRESS  0x18    // the 7-bit address 
//#define _SDA                  4
//#define _SCL                  5

//----- Name  to  Slot
#define LED1        0
#define LED2        1
#define LED3        2
#define LED4        3
#define SW1         4
#define SW2         5
#define SW3         6
#define SW4         7

#include <I2C_ADW0720.h>


I2CADW0720    ExtPlusBrd1; // Create instance of the I2CADW0720 object

byte          whoAmI;
byte          majorRelease, minorRelease;
uint32_t      builtinLedTimer;
int8_t        inMode = 0;


//===========================================================================================
void handleInput()
{
  byte statusReg, sysStatus, slotModes;
  static bool sLed1, sLed2, sLed3, sLed4;
  
  sysStatus = ExtPlusBrd1.getSysStatus();
  if (sysStatus == 0) return;

  #ifdef VERBOSE
    Serial.print("sysStatus: ");
    ExtPlusBrd1.printRegister(&Serial, sizeof(sysStatus), &sysStatus);
    slotModes = ExtPlusBrd1.getSlotModes();
    Serial.print(", slotModes: ");
    ExtPlusBrd1.printRegister(&Serial, sizeof(slotModes), &slotModes);
    Serial.println();
  #endif
  
  if ( BIT_IS_HIGH(sysStatus, SW1) )  
  {
    statusReg = ExtPlusBrd1.getSlotStatus(SW1);
    #ifdef VERBOSE
      Serial.print("sysStatus: ");
      ExtPlusBrd1.printRegister(&Serial, sizeof(sysStatus), &sysStatus);
      Serial.print(", slotStatus[SW1(slot "); Serial.print(SW1); Serial.print(")]: ");
      ExtPlusBrd1.printRegister(&Serial, 1, &statusReg);
      Serial.println();
    #endif
    
    if (ExtPlusBrd1.isSlotPressed(SW1) ) 
    {
      Serial.println(F("(SW1)-------> Button Pressed"));
      ExtPlusBrd1.setOutputToggle(LED1, HIGH, 200);
    }
    if (ExtPlusBrd1.isSlotQuickReleased(SW1) ) 
    {
      Serial.println(F("(SW1)-------> Quick Release "));
      if (sLed1) 
      {
        Serial.println("\t setOutputToggle(LED1, LOW, 0);");
        ExtPlusBrd1.setOutputToggle(LED1, LOW, 0);
        sLed1 = false;
      }
      else
      {
        Serial.println("\t setOutputToggle(LED1, HIGH, 5000);");
        ExtPlusBrd1.setOutputToggle(LED1, HIGH, 5000);
        sLed1 = true;
      }
    }
    if (ExtPlusBrd1.isSlotMidReleased(SW1) ) 
    {
      Serial.println(F("(SW1)-------> Mid Release   "));
      Serial.println("\t setOutputPulse(LED1, 50, 50, 0);");
      ExtPlusBrd1.setOutputPulse(LED1, 50, 50, 0);
    }
    if (ExtPlusBrd1.isSlotLongReleased(SW1) ) 
    {
      Serial.println(F("(SW1)-------> Long Release  "));
      Serial.println("\t setOutputPulse(LED1, 100, 100, 0);");
      ExtPlusBrd1.setOutputPulse(LED1, 100, 100, 0);
    }
  } // bitRead(slot0)
  
  if ( (sysStatus & _BV(SW2)) != 0 )  
  {
    statusReg = ExtPlusBrd1.getSlotStatus(SW2);
    //Serial.print("slotStatus[SW2(slot "); Serial.print(SW2); Serial.print(")]: ");
    //ExtPlusBrd1.printRegister(&Serial, 1, &statusReg);
    //Serial.println();

    if (ExtPlusBrd1.isSlotPressed(SW2) ) 
    {
      Serial.println(F("(SW2)-------> Button Pressed"));
      ExtPlusBrd1.setOutputToggle(LED2, HIGH, 200);
    }
    if (ExtPlusBrd1.isSlotQuickReleased(SW2) ) 
    {
      Serial.println(F("(SW2)-------> Quick Release "));
      Serial.println("\t setOutputToggle(LED2, HIGH, 5000);");
      ExtPlusBrd1.setOutputToggle(LED2, HIGH, 5000);
    }
    if (ExtPlusBrd1.isSlotMidReleased(SW2) ) 
    {
      Serial.println(F("(SW2)-------> Mid Release   "));
      Serial.println("\t setOutputPulse(LED2, 100, 150, 0);");
      ExtPlusBrd1.setOutputPulse(LED2, 100, 150, 0);
    }
    if (ExtPlusBrd1.isSlotLongReleased(SW2) ) 
    {
      Serial.println(F("(SW2)-------> Long Release  "));
      Serial.println("\t setOutputPulse(LED2, 300, 500, 0);");
      ExtPlusBrd1.setOutputPulse(LED2, 300, 500, 0);
    }
  } // bitRead(SW2)
  
  if ( (sysStatus & _BV(SW3)) != 0 )  
  {
    statusReg = ExtPlusBrd1.getSlotStatus(SW3);
    //Serial.print("slotStatus[SW3(slot "); Serial.print(SW3); Serial.print(")]: ");
    //ExtPlusBrd1.printRegister(&Serial, 1, &statusReg);
    //Serial.println();

    if (ExtPlusBrd1.isSlotPressed(SW3) ) 
    {
      Serial.println(F("(SW3)-------> Button Pressed"));
      ExtPlusBrd1.setOutputToggle(LED3, HIGH, 200);
    }
    if (ExtPlusBrd1.isSlotQuickReleased(SW3) ) 
    {
      Serial.println(F("(SW3)-------> Quick Release "));
      Serial.println("\t setOutputToggle(LED3, HIGH, 5000);");
      ExtPlusBrd1.setOutputToggle(LED3, HIGH, 5000);
    }
    if (ExtPlusBrd1.isSlotMidReleased(SW3) ) 
    {
      Serial.println(F("(SW3)-------> Mid Release   "));
      Serial.println("\t setOutputPulse(LED3, 700, 900, 0);");
      ExtPlusBrd1.setOutputPulse(LED3, 700, 900, 0);
    }
    if (ExtPlusBrd1.isSlotLongReleased(SW3) ) 
    {
      Serial.println(F("(SW3)-------> Long Release  "));
      Serial.println("\t setOutputPulse(LED3, 1000, 1200, 0);");
      ExtPlusBrd1.setOutputPulse(LED3, 1000, 1200, 0);
    }
  } // bitRead(SW3)
  
  if ( (sysStatus & _BV(SW4)) != 0 )  
  {
    statusReg = ExtPlusBrd1.getSlotStatus(SW4);
    //Serial.print("slotStatus[SW4(slot "); Serial.print(SW4); Serial.print(")]: ");
    //ExtPlusBrd1.printRegister(&Serial, 1, &statusReg);
    //Serial.println();
    
    if (ExtPlusBrd1.isSlotPressed(SW4) ) 
    {
      Serial.println(F("(SW4)-------> Button Pressed"));
      ExtPlusBrd1.setOutputToggle(LED4, HIGH, 200);
    }
    if (ExtPlusBrd1.isSlotQuickReleased(SW4) ) 
    {
      Serial.println(F("(SW4)-------> Quick Release "));
      Serial.println("\t setOutputToggle(LED4, HIGH, 5000);");
      ExtPlusBrd1.setOutputToggle(LED4, HIGH, 5000);
    }
    if (ExtPlusBrd1.isSlotMidReleased(SW4) ) 
    {
      Serial.println(F("(SW4)-------> Mid Release   "));
      Serial.println("\t setOutputPulse(LED4, 700, 1000, 0);");
      ExtPlusBrd1.setOutputPulse(LED4, 700, 1000, 0);
    }
    if (ExtPlusBrd1.isSlotLongReleased(SW4) ) 
    {
      Serial.println(F("(SW4)-------> Long Release  "));
      Serial.println("\t setOutputPulse(LED4, 1000, 1200, 0);");
      ExtPlusBrd1.setOutputPulse(LED4, 1000, 1200, 0);
    }
  } // bitRead(SW4)

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

  if (ExtPlusBrd1.begin(Wire, I2C_DEFAULT_ADDRESS)) 
  {
    majorRelease = ExtPlusBrd1.getMajorRelease();
    minorRelease = ExtPlusBrd1.getMinorRelease();
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

  Serial.print("debounceTime[");  Serial.print(ExtPlusBrd1.getDebounceTime());  Serial.println("]");
  Serial.print("MidPressTime[");  Serial.print(ExtPlusBrd1.getMidPressTime());  Serial.println("]");
  Serial.print("LongPressTime["); Serial.print(ExtPlusBrd1.getLongPressTime()); Serial.println("]");

  byte slotModes = ExtPlusBrd1.getSlotModes();
  Serial.print("on board slotModes: ");
  ExtPlusBrd1.printRegister(&Serial, sizeof(slotModes), &slotModes);
  Serial.println();
  /**
  for (int i=0; i<8; i++)
  {
    Serial.print("Bit ["); Serial.print(i); Serial.print("] is ");
    if (BIT_IS_HIGH(slotModes, i))  
          Serial.println("HIGH");
    else  Serial.println("LOW");
  }
  **/
  
  //slotModes = ExtPlusBrd1.getSlotModes();
  //Serial.print("slotModes: ");
  //ExtPlusBrd1.printRegister(&Serial, 1, &slotModes);
  //Serial.println();
  Serial.println(F("Now set slot modes for AND0720 type-1 board:"));
  ExtPlusBrd1.setModeInput(SW1);
  ExtPlusBrd1.setModeInput(SW2);
  ExtPlusBrd1.setModeInput(SW3);
  ExtPlusBrd1.setModeInput(SW4);
  ExtPlusBrd1.setModeOutput(LED1);
  ExtPlusBrd1.setModeOutput(LED2);
  ExtPlusBrd1.setModeOutput(LED3);
  ExtPlusBrd1.setModeOutput(LED4);
  slotModes = ExtPlusBrd1.getSlotModes();
  Serial.print("slotModes: ");
  ExtPlusBrd1.printRegister(&Serial, 1, &slotModes);
  Serial.println();
  
  slotModes = ExtPlusBrd1.getSlotModes();
  Serial.print("new slotModes: ");
  ExtPlusBrd1.printRegister(&Serial, sizeof(slotModes), &slotModes);
  Serial.println();
  for (int i=0; i<8; i++)
  {
    Serial.print("Bit ["); Serial.print(i); Serial.print("] is ");
    if (BIT_IS_HIGH(slotModes, i))  
          Serial.println("HIGH (input)");
    else  Serial.println("LOW  (output)");
  }

  //ExtPlusBrd1.setModeOutput(LED1);
  Serial.println("> setOutputToggle(LED1, HIGH, 4000)");
  ExtPlusBrd1.setOutputToggle(LED1, HIGH, 4000);
  delay(500);
  //ExtPlusBrd1.setModeOutput(LED2);
  Serial.println("> setOutputToggle(LED2, HIGH, 3000)");
  ExtPlusBrd1.setOutputToggle(LED2, HIGH, 3000);
  delay(500);
  //ExtPlusBrd1.setModeOutput(LED3);
  Serial.println("> setOutputToggle(LED3, HIGH, 2000)");
  ExtPlusBrd1.setOutputToggle(LED3, HIGH, 2000);
  delay(500);
  //ExtPlusBrd1.setModeOutput(LED4);
  Serial.println("> setOutputToggle(LED4, HIGH, 1000)");
  ExtPlusBrd1.setOutputToggle(LED4, HIGH, 1000);

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
