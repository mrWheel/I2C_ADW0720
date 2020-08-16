/*
**    Program : I2C_ADW0720_Tiny841_Slave
**    Date    : 16-08-2020
*/
#define _MAJOR_RELEASE  1
#define _MINOR_RELEASE  4
/*
**    Copyright (c) 2020 Willem Aandewiel
**
**    TERMS OF USE: MIT License. See bottom of file.
***************************************************************************
*/

/*
* You need https://github.com/SpenceKonde/ATTinyCore to compile this source
*
* Settings ATtiny841:
*    Board:                 "ATtiny 441/841 (No bootloader)"
*    Chip:                  "Attiny841"
*    Clock:                 "8 MHz (internal)"
*    B.O.D. Level:          "B.O.D. Enabled (1.8v)"
*    B.O.D. Mode (active):  "B.O.D. Disabled"
*    B.O.D. Mode (sleep):   "B.O.D. Disabled"
*    Save EEPROM:           "EEPROM retained"
*    Pin Mapping:           "Clockwise (like Rev. D boards)"
*    LTO (1.611+ only):     "Enabled"
*    Wire Modes:            "Slave Only"
*    tinyNeoPixel Port:     "Port A (CW:0~7, CCW:3~10)"
*    millis()/micros():     "Enabled"
*    
* Alternative ATtiny84
*    Board:               "ATtiny24/44/48"
*    Chip:                "Attiny84"
*    Clock:               "8 MHz (internal)"
*    B.O.D. Level:        "B.O.D. Disabled"
*    Save EEPROM:         "EEPROM retained"
*    Pin Mapping:         "Clockwise (like Damellis code)"
*    LTO (1.6.11+ only):  "Enabled"
*    tiny NeoPixel Port:  "Port A (CW:0~7,CCW:3~10)"
*    millis()/micros():   "Enabled"

*/

#include <Wire.h>
#include <EEPROM.h>

#define SETBIT(regByte,      bit)  (regByte) |=  (1 << (bit))
#define CLEARBIT(regByte,    bit)  (regByte) &= ~(1 << (bit))
#define BIT_IS_HIGH(regByte, bit)  ((regByte) & (1<<(bit)))
#define BIT_IS_LOW(regByte,  bit)  (!(BIT_IS_HIGH(regByte, bit)))

#define _I2C_DEFAULT_ADDRESS  0x18

/*
* see: 
* https://www.nongnu.org/avr-libc/user-manual/using_tools.html
* for processor names
*/

#if defined (__AVR_ATtiny841__) || defined (__AVR_ATtiny84__)
  #warning ATtiny841 or ATtiny84
  #define Debug(...)
  #define Debugln(...)
  #define DebugFlush(...)

#elif defined (__AVR_ATmega328P__)
  #define Debug(...)      Serial.print(__VA_ARGS__);
  #define Debugln(...)    Serial.println(__VA_ARGS__);
  #define DebugFlush(...) Serial.flush(__VA_ARGS__);

#else
  #Error: unknow Processor type
  
#endif

//------ commands --------------------------------------------------------------
enum  {  CMD_READCONF, CMD_WRITECONF, CMD_DUM2, CMD_DUM3, CMD_DUM4, CMD_DUM5
       , CMD_DUM6,  CMD_REBOOT
      };

//------ slotStatus bit's ------------------------------------------------------
enum  {  SLT_PRESSED_BIT        // bit 0  Input 1 if Button pressed
       , SLT_QUICKRELEASE_BIT   // bit 1  Input 1 if Button Quick Released
       , SLT_MIDRELEASE_BIT     // bit 2  Input 1 if Button Mid Released
       , SLT_LONGRELEASE_BIT    // bit 3  Input 1 if Button Long Released
       , SLT_TOGGLE_BIT         // bit 4  output 1 in Toggle mode
       , SLT_PULSE_BIT          // bit 5  output 1 in Pulse mode
       , SLT_PWM_BIT            // bit 6  output 1 in PWM mode
       , SLT_HIGH_BIT           // bit 7  output 1 bit is HIGH
      };
//------ finite states Outputs -------------------------------------------------
enum  {  OUTPUT_DEFAULT, OUTPUT_PULSE_ON, OUTPUT_PULSE_OFF, OUTPUT_PWM_SET, OUTPUT_PWM };
//------ finite states Inputs --------------------------------------------------
enum  {  BTN_INIT, BTN_FIRST_PRESS, BTN_IS_PRESSED, BTN_FIRST_RELEASE, BTN_IS_RELEASED };


struct registerLayout {
  byte      address;            // 0x00 (R/W)
  byte      majorRelease;       // 0x01 (RO)
  byte      minorRelease;       // 0x02 (RO)
  byte      sysStatus;          // 0x03 (RO)
  byte      slotStatus[8];      // 0x04 (RO) 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B
  uint8_t   slotModes;          // 0x0C (RO) -->> _SLOTMODES must be the same offset
  uint8_t   debounceTime;       // 0x0D (R/W)
  uint16_t  midPressTime;       // 0x0E (R/W) - 2 bytes 0x0E 0x0F
  uint16_t  longPressTime;      // 0x10 (R/W) - 2 bytes 0x10 0x11
  byte      tmpSlotNr;          // 0x12 (R/W) -->> _TMPSLOTNR must be the same offset
  byte      tmpOutputFunc;      // 0x13 (R/W)
  uint8_t   tmpPWMvalue;        // 0x14 (R/W)
  uint16_t  tmpPulseHighTime;   // 0x15 (R/W) - 2 bytes 0x15 0x16
  uint16_t  tmpPulseLowTime;    // 0x17 (R/W) - 2 bytes 0x17 0x18
  uint16_t  tmpStateDuration;   // 0x19 (R/W) - 2 bytes 0x19 0x1A
  uint8_t   modeSettings;       // 0x1B (R/W) -->> _MODESETTINGS must be the same offset <<--
  byte      filler[4];          // 0x1C -> 0x1C, 0x1D, 0x1E, 0x1F
};

#define _TMPSLOTNR      0x12
#define _SLOTMODES      0x0C
#define _MODESETTINGS   0x1B
#define _CMD_REGISTER   0xF0  // not a real register!

//These are the defaults for all settings
volatile registerLayout registerStack = {
  .address =    _I2C_DEFAULT_ADDRESS,     // 0x00
  .majorRelease =     _MAJOR_RELEASE,     // 0x01
  .minorRelease =     _MINOR_RELEASE,     // 0x02
  .sysStatus =                     0,     // 0x03
  .slotStatus =    {0,0,0,0,0,0,0,0},     // 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B
  .slotModes =                  0xFF,     // 0x0C   -> default all Slots INPUT
  .debounceTime =                  5,     // 0x0D
  .midPressTime =                500,     // 0x0E 2 --> 0x0E 0x0F
  .longPressTime =              1500,     // 0x10 2 --> 0x10 0x11
  .tmpSlotNr =                     0,     // 0x12
  .tmpOutputFunc =                 0,     // 0x13
  .tmpPWMvalue =                   0,     // 0x14
  .tmpPulseHighTime =              0,     // 0x15 2 --> 0x15 0x16
  .tmpPulseLowTime =               0,     // 0x17 2 --> 0x17 0x18
  .tmpStateDuration =              0,     // 0x19 2 --> 0x19 0x1A
  .modeSettings =               0x00,     // 0x1B
  .filler =  {0xFF, 0xFF, 0xFF, 0xFF}     // 0x1C 4 --> 0x1C, 0x1D, 0x1E, 0x1F
};

//Cast 32bit address of the object registerStack with uint8_t so we can increment the pointer
uint8_t *registerPointer = (uint8_t *)&registerStack;

volatile byte registerNumber; 

static uint8_t    prevNextCode = 0;
static uint16_t   store=0;

struct slotFields {
  byte      state;
  bool      isHigh;
  uint8_t   pwmValue;
  uint32_t  durationTimer;
  uint16_t  duration;
  uint32_t  highTimer;
  uint16_t  highTime;
  uint32_t  lowTimer;
  uint16_t  lowTime;
  bool      pressedStateSend;
  uint32_t  bounceTimer;
};


slotFields  slot[8]; 


//==========================================================================

//==========================================================================
void setSlotAsOutput(uint8_t slotNr)
{
  Debug("setSlotAsOutput("); Debug(slotNr); Debugln(") ");
  DebugFlush();
  
  switch(slotNr)
  {
    #if defined (__AVR_ATtiny841__) || defined (__AVR_ATtiny84__)
      case 0:   pinMode(PIN_PB0, OUTPUT); break;
      case 1:   pinMode(PIN_PB1, OUTPUT); break;
      case 2:   pinMode(PIN_PB2, OUTPUT); break;
      case 3:   pinMode(PIN_PA7, OUTPUT); break;
      case 4:   pinMode(PIN_PA3, OUTPUT); break;
      case 5:   pinMode(PIN_PA2, OUTPUT); break;
      case 6:   pinMode(PIN_PA1, OUTPUT); break;
      case 7:   pinMode(PIN_PA0, OUTPUT); break;
    #endif
    #if defined (__AVR_ATmega328P__)
      case 0:   pinMode( 8, OUTPUT); break;
      case 1:   pinMode( 9, OUTPUT); break;
      case 2:   pinMode(10, OUTPUT); break;
      case 3:   pinMode(11, OUTPUT); break;
      case 4:   pinMode(A0, OUTPUT); break;
      case 5:   pinMode(A1, OUTPUT); break;
      case 6:   pinMode(A2, OUTPUT); break;
      case 7:   pinMode(A3, OUTPUT); break;
    #endif
  }
  CLEARBIT(registerStack.slotModes, slotNr);
  registerStack.slotStatus[slotNr] = 0;
  slot[slotNr].state = OUTPUT_DEFAULT; 

} // setSlotAsOutput()

//==========================================================================
void setSlotAsInput(uint8_t slotNr)
{
  Debug("setSlotAsInput("); Debug(slotNr); Debugln(") ");
  DebugFlush();
  
  switch(slotNr)
  {
    #if defined (__AVR_ATtiny841__) || defined (__AVR_ATtiny84__)
      case 0:   pinMode(PIN_PB0, INPUT_PULLUP); break;
      case 1:   pinMode(PIN_PB1, INPUT_PULLUP); break;
      case 2:   pinMode(PIN_PB2, INPUT_PULLUP); break;
      case 3:   pinMode(PIN_PA7, INPUT_PULLUP); break;
      case 4:   pinMode(PIN_PA3, INPUT_PULLUP); break;
      case 5:   pinMode(PIN_PA2, INPUT_PULLUP); break;
      case 6:   pinMode(PIN_PA1, INPUT_PULLUP); break;
      case 7:   pinMode(PIN_PA0, INPUT_PULLUP); break;
    #endif
    #if defined (__AVR_ATmega328P__)
      case 0:   pinMode( 8, INPUT_PULLUP); break;
      case 1:   pinMode( 9, INPUT_PULLUP); break;
      case 2:   pinMode(10, INPUT_PULLUP); break;
      case 3:   pinMode(11, INPUT_PULLUP); break;
      case 4:   pinMode(A0, INPUT_PULLUP); break;
      case 5:   pinMode(A1, INPUT_PULLUP); break;
      case 6:   pinMode(A2, INPUT_PULLUP); break;
      case 7:   pinMode(A3, INPUT_PULLUP); break;
    #endif
  }
  SETBIT(registerStack.slotModes, slotNr); 
  registerStack.slotStatus[slotNr] = 0;
  
} // setSlotAsInput()

//==========================================================================
bool slotRead(uint8_t slotNr)
{
  switch(slotNr)
  {
    #if defined (__AVR_ATtiny841__) || defined (__AVR_ATtiny84__)
      case 0:   //--- PB0, slot0, D10
                return(BIT_IS_HIGH(PINB, 0));
                break;
      case 1:   //--- PB1, slot1, D9
                return(BIT_IS_HIGH(PINB, 1));
                break;
      case 2:   //--- PB2, slot2, D8
                return(BIT_IS_HIGH(PINB, 2));
                break;
      case 3:   //--- PA7, slot3, D7
                return(BIT_IS_HIGH(PINA, 7));
                break;
      case 4:   //--- PA3, slot4, D3
                return(BIT_IS_HIGH(PINA, 3));
                break;
      case 5:   //--- PA2, slot5, D2
                return(BIT_IS_HIGH(PINA, 2));
                break;
      case 6:   //--- PA1, slot6, D1
                return(BIT_IS_HIGH(PINA, 1));
                break;
      case 7:   //--- PA0, slot7, D0
                return(BIT_IS_HIGH(PINA, 0));
                break;
      default:  return false;
    #endif
    
    #if defined (__AVR_ATmega328P__)
      case 0:   return(digitalRead( 8)); break;
      case 1:   return(digitalRead( 9)); break;
      case 2:   return(digitalRead(10)); break;
      case 3:   return(digitalRead(11)); break;
      case 4:   return(digitalRead(A0)); break;
      case 5:   return(digitalRead(A1)); break;
      case 6:   return(digitalRead(A2)); break;
      case 7:   return(digitalRead(A3)); break;
      default:  return false;
    #endif
  }
} // slotRead()

//==========================================================================
void slotWrite(uint8_t slotNr, bool newState)
{
  switch(slotNr)
  {
    #if defined (__AVR_ATtiny841__) || defined (__AVR_ATtiny84__)
      case 0:   //--- PB0, slot0, D10
                digitalWrite(PIN_PB0, newState);
                break;
      case 1:   //--- PB1, slot1, D9
                digitalWrite(PIN_PB1, newState);
                break;
      case 2:   //--- PB2, slot2, D8
                digitalWrite(PIN_PB2, newState);
                break;
      case 3:   //--- PA7, slot3, D7
                digitalWrite(PIN_PA7, newState);
                break;
      case 4:   //--- PA3, slot4, D3
                digitalWrite(PIN_PA3, newState);
                break;
      case 5:   //--- PA2, slot5, D2
                digitalWrite(PIN_PA2, newState);
                break;
      case 6:   //--- PA1, slot6, D1
                digitalWrite(PIN_PA1, newState);
                break;
      case 7:   //--- PA0, slot7, D0
                digitalWrite(PIN_PA0, newState);
                break;
      default:  return;
    #endif

    #if defined (__AVR_ATmega328P__)
      case 0:   digitalWrite( 8, newState); break;
      case 1:   digitalWrite( 9, newState); break;
      case 2:   digitalWrite(10, newState); break;
      case 3:   digitalWrite(11, newState); break;
      case 4:   digitalWrite(A0, newState); break;
      case 5:   digitalWrite(A1, newState); break;
      case 6:   digitalWrite(A2, newState); break;
      case 7:   digitalWrite(A3, newState); break;
      default:  return;
    #endif
  }
  slot[slotNr].state = OUTPUT_DEFAULT;
  if (newState)
        SETBIT(registerStack.slotStatus[slotNr],   SLT_HIGH_BIT);
  else  CLEARBIT(registerStack.slotStatus[slotNr], SLT_HIGH_BIT);
  
} // slotWrite()

//==========================================================================
void slotWritePWM(uint8_t slotNr, uint8_t pwmValue)
{
  switch(slotNr)
  {
    #if defined (__AVR_ATtiny841__) || defined (__AVR_ATtiny84__)
      case 2:   //--- PB2, slot2, D8
                analogWrite(PIN_PB2, pwmValue);
                slot[slotNr].state = OUTPUT_PWM_SET;
                break;
      case 3:   //--- PA7, slot3, D7
                analogWrite(PIN_PA7, pwmValue);
                slot[slotNr].state = OUTPUT_PWM_SET;
                break;
      case 4:   //--- PA3, slot4, D3
                analogWrite(PIN_PA3, pwmValue);
                slot[slotNr].state = OUTPUT_PWM_SET;
                break;
      default:  slot[slotNr].state = OUTPUT_DEFAULT;
                return;
    #endif
    #if defined (__AVR_ATmega328P__)
      case 2:   analogWrite(10, pwmValue); 
                slot[slotNr].state = OUTPUT_PWM_SET; 
                break;
      case 3:   analogWrite(11, pwmValue); 
                slot[slotNr].state = OUTPUT_PWM_SET; 
                break;
      case 4:   analogWrite(A0, pwmValue); 
                slot[slotNr].state = OUTPUT_PWM_SET; 
                break;
      default:  slot[slotNr].state = OUTPUT_DEFAULT;
                return;
    #endif
  }
  if (pwmValue > 0)
        SETBIT(registerStack.slotStatus[slotNr],   SLT_HIGH_BIT);
  else  CLEARBIT(registerStack.slotStatus[slotNr], SLT_HIGH_BIT);
  
} // slotWritePWM()

//==========================================================================
void reBoot()
{
  Wire.end();
  //-- restart slave
  setup();

} //  reBoot()


//==========================================================================
void activateToggleOutput(uint8_t slotNr, bool setOutputHigh, uint16_t stateDuration)
{
  Debug("activateToggleOutput("); Debug(slotNr); Debugln(")..");DebugFlush();

  if (slotNr < 0 || slotNr > 7) return;

  SETBIT(registerStack.slotStatus[slotNr], SLT_TOGGLE_BIT);
  if (setOutputHigh)
        SETBIT(registerStack.slotStatus[slotNr],   SLT_HIGH_BIT);
  else  CLEARBIT(registerStack.slotStatus[slotNr], SLT_HIGH_BIT);
  
  slot[slotNr].durationTimer = millis() + stateDuration;
  slot[slotNr].duration      = stateDuration;
  slot[slotNr].isHigh        = setOutputHigh;
  slot[slotNr].pwmValue      = 0;
  slot[slotNr].highTime      = 0;
  slot[slotNr].highTimer     = 0;
  slot[slotNr].lowTime       = 0;
  slot[slotNr].lowTimer      = 0;
  slot[slotNr].state         = OUTPUT_DEFAULT;
  
} // activateToggleOutput()


//==========================================================================
void activatePulseOutput(uint8_t slotNr, uint16_t setHighTime
                                       , uint16_t setLowTime
                                       , uint16_t stateDuration)
{
  Debug("activatePulseOutput("); Debug(slotNr);  Debugln("):"); DebugFlush();

  if (slotNr < 0 || slotNr > 7) return;
  
  SETBIT(registerStack.slotStatus[slotNr], SLT_PULSE_BIT);
  
  slot[slotNr].durationTimer = millis() + stateDuration;
  slot[slotNr].duration      = stateDuration;
  slot[slotNr].pwmValue      = 0;
  slot[slotNr].highTime      = setHighTime;
  slot[slotNr].highTimer     = millis() + setHighTime;
  slot[slotNr].lowTime       = setLowTime;
  slot[slotNr].lowTimer      = millis() + setLowTime;
  slot[slotNr].state         = OUTPUT_PULSE_ON;

  Debugln(slot[slotNr].highTime); DebugFlush();
  Debugln(slot[slotNr].lowTime);  DebugFlush();
  Debugln(slot[slotNr].duration); DebugFlush();
  Debugln(slot[slotNr].state);    DebugFlush();

} // activatePulseOutput()


//==========================================================================
void activatePwmOutput(uint8_t slotNr, uint8_t pwmVal
                                     , uint16_t stateDuration)
{
  Debug("activatePwmOutput("); Debug(slotNr);  Debugln("):"); DebugFlush();

  if (slotNr != 2 && slotNr != 3 && slotNr != 4) return;

  if (pwmVal > 0)
  {
    SETBIT(registerStack.slotStatus[slotNr], SLT_PWM_BIT);
    slot[slotNr].state      = OUTPUT_PWM_SET;
  }
  else
  {
    activateToggleOutput(slotNr, LOW, stateDuration);
    slotWrite(slotNr, LOW);
    return;
  }
  
  slot[slotNr].durationTimer = millis() + stateDuration;
  slot[slotNr].duration      = stateDuration;
  slot[slotNr].isHigh        = false;
  slot[slotNr].pwmValue      = pwmVal;
  slot[slotNr].highTime      = 0;
  slot[slotNr].highTimer     = 0;
  slot[slotNr].lowTime       = 0;
  slot[slotNr].lowTimer      = 0;

  Debugln(slot[slotNr].pwmValue); DebugFlush();
  Debugln(slot[slotNr].duration); DebugFlush();
  Debugln(slot[slotNr].state);    DebugFlush();

} // activatePwmOutput()


//==========================================================================
void setOutputFunc(uint8_t slotNr, byte outputFunc
                             , uint8_t  pwmValue
                             , uint16_t pulseHighTime
                             , uint16_t pulseLowTime
                             , uint16_t stateDuration)
{
  bool setOutputHigh;
  uint8_t func = 0;

  Debug("outputFunc: "); showRegister(1, &outputFunc); Debugln(); DebugFlush();

  if (slotNr < 0 || slotNr > 7) return;

  //-- first clear the slotStatus ----
  registerStack.slotStatus[slotNr] = 0;

  if ( BIT_IS_HIGH(outputFunc, SLT_TOGGLE_BIT) )   // Function is On/Off
  {
    Debug("setOutputFunc() slot["); Debug(slotNr); Debug("]==> ");
    setOutputHigh = BIT_IS_HIGH(outputFunc, SLT_HIGH_BIT); // High or Low
    Debugln(setOutputHigh ? "HIGH" : "LOW");
    activateToggleOutput(slotNr, setOutputHigh, stateDuration);
  }
  else if( BIT_IS_HIGH(outputFunc, SLT_PULSE_BIT) )   // Function is Pulse
  {
    Debugln("setOutputFunc() slot["); Debug(slotNr); Debugln("]");
    activatePulseOutput(slotNr, pulseHighTime, pulseLowTime, stateDuration);
  }
  else if( BIT_IS_HIGH(outputFunc, SLT_PWM_BIT) )   // Function is PWM
  {
    Debug("setOutputFunc() slot["); Debug(slotNr); Debug("] ");
    Debug(", PWM value["); Debug(pwmValue); Debugln("] ");
    activatePwmOutput(slotNr, pwmValue, stateDuration);
  }

} // setOutputFunc()


//==========================================================================
void updateSlotModes()
{
  Debug("updateSlotModes(): slotMode --> ");
  showRegister(1, &registerStack.slotModes);
  Debugln();
  for (int8_t slt=0; slt<8; slt++)
  {
    Debug("set slot["); Debug(slt); 
    if ( BIT_IS_HIGH(registerStack.slotModes, slt) )   //-- input pin
    {
      Debugln("] to input");
      setSlotAsInput(slt);
    }
    else
    {
      Debugln("] to output");
      setSlotAsOutput(slt);
    }
    CLEARBIT(registerStack.sysStatus, slt);
  }
  Debug("updateSlotModes(): slotMode ==> ");
  showRegister(1, &registerStack.slotModes);
  Debugln();
  
} // updateSlotModes()


//==========================================================================
void setup()
{
#if defined (__AVR_ATmega328P__)
  Serial.begin(115200);
  while(!Serial) {/* wait a bit */}
  Debugln("\n\rI2C_ADW0720_Tiny841_Slave startup ..\r\n");
#endif

  
  readConfig();

  updateSlotModes();
  
  startI2C();

  /***
  for (int8_t s=0; s<8; s++)
  {
    if (BIT_IS_LOW(registerStack.slotModes, s))
    {
      activateToggleOutput(s, HIGH, 500);
      delay(50);
    }
  }
  ***/
  registerStack.sysStatus = 0;
  for(int8_t r=0; r<7; r++)  registerStack.slotStatus[r] = 0;
  
#if defined (__AVR_ATmega328P__)
  Debugln("slotModes: ");
  showRegister(1, &registerStack.slotModes);
  Debugln();
  Debugln("[<stat7-> <stat6-> <stat5-> <stat4-> <stat3-> <stat2-> <stat1-> <stat0->]");
  showRegister(8, &registerStack.slotStatus[0]);
  Debugln();
  Debug("sysStat:");
  showRegister(1, &registerStack.sysStatus);
  Debugln();
  Debug("slotModes:");
  showRegister(1, &registerStack.slotModes);
  Debugln();
  for (int8_t s=0; s<8; s++)
  {
    Debug("Slot["); Debug(s);
    if ( BIT_IS_HIGH(registerStack.slotModes, s) )  
          {Debugln("] => input"); }
    else  {Debugln("] => output");}
  }

  Debugln("\n\rI2C_ADW0720_Tiny841_Slave waiting for your command's ....\r\n");

#endif
  
} // setup()


//==========================================================================
void loop()
{
  /*
  * mail Loop takes about 142.85 micro seconds to complete with 
  * all eight slots serviced.
  */
  for (int8_t s=0; s<8; s++)
  {
    //-- test if slot is set as input (default) 
    if (BIT_IS_HIGH(registerStack.slotModes, s) )  
    {
      handleInput(s);
    }
    else  // no, its an output slot!
    {
      handleOutput(s);
    }
  } // for ..

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
