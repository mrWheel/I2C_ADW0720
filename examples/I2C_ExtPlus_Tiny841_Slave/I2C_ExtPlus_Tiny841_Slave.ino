/*
**    Program : I2C_ExtPlus_Tiny841_Slave
**    Date    : 27-07-2020
*/
#define _MAJOR_VERSION  1
#define _MINOR_VERSION  1
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

#define SETBIT(regByte, bit)     (regByte) |=  (1 << (bit))
#define CLEARBIT(regByte, bit)   (regByte) &= ~(1 << (bit))
#define BIT_IS_HIGH(regByte, bit)  ((regByte) & (1<<(bit)))
#define BIT_IS_LOW(regByte, bit)   (!((regByte) & (1<<(bit))))

#define _I2C_DEFAULT_ADDRESS  0x18

#define _LED1       0
#define _LED2       1
#define _LED3       2
#define _LED4       3
#define _SW1        4
#define _SW2        5
#define _SW3        6
#define _SW4        7

/*
* see: 
* https://www.nongnu.org/avr-libc/user-manual/using_tools.html
* for processor names
*/

#if defined (__AVR_ATtiny841__) || defined (__AVR_ATtiny84__)
  #warning ATtiny841 or ATtiny84
  #define Debug(...)
  #define Debugln(...)
  #define DebugFlash(...)

#elif defined (__AVR_ATmega328P__)
  #define Debug(...)      Serial.print(__VA_ARGS__);
  #define Debugln(...)    Serial.println(__VA_ARGS__);
  #define DebugFlash(...) Serial.flush(__VA_ARGS__);

#else
  #Error: unknow Processor type
  
#endif


//------ Led Functions ---------------------------------------------------------
//enum  {  FUNC_ON, FUNC_TYPE_ONOF, FUNCT_TYPE_BLINK };

//------ commands --------------------------------------------------------------
enum  {  CMD_READCONF, CMD_WRITECONF, CMD_DUM2, CMD_DUM3, CMD_DUM4, CMD_DUM5
       , CMD_DUM6,  CMD_REBOOT
      };
//------ slotStatus bit's ------------------------------------------------------
enum  {  SLT_PRESSED_BIT, SLT_QUICKRELEASE_BIT, SLT_MIDRELEASE_BIT, SLT_LONGRELEASE_BIT 
       , SLT_HIGH_BIT, SLT_TOGGLE_BIT, SLT_PULSE_BIT };
       //------ finite states Outputs ------------------------------------------
enum  {  OUTPUT_DEFAULT, OUTPUT_PULSE_ON, OUTPUT_PULSE_OFF};
//------ finite states Inputs --------------------------------------------------
enum  {  BTN_INIT, BTN_FIRST_PRESS, BTN_IS_PRESSED, BTN_FIRST_RELEASE, BTN_IS_RELEASED };


struct registerLayout {
  byte      sysStatus;          // 0x00
  byte      slotStatus[8];      // 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
  byte      address;            // 0x09
  byte      majorRelease;       // 0x0A
  byte      minorRelease;       // 0x0B
  uint8_t   debounceTime;       // 0x0C
  uint16_t  midPressTime;       // 0x0D - 2 bytes 0x0D 0x0E
  uint16_t  longPressTime;      // 0x0F - 2 bytes 0x0F 0x10
  byte      tmpSlotNr;          // 0x11 -->> _TMPSLOTNR must be the same offset
  byte      tmpOutputFunc;      // 0x12
  uint16_t  tmpPulseHighTime;   // 0x13 - 2 bytes 0x13 0x14
  uint16_t  tmpPulseLowTime;    // 0x15 - 2 bytes 0x15 0x16
  uint16_t  tmpStateDuration;   // 0x17 - 2 bytes 0x17 0x18
  uint8_t   slotModes;          // 0x19 -->> _SLOTMODES must be the same offset
  uint8_t   modeSettings;       // 0x1A -->> _MODESETTINGS must be the same offset <<--
  byte      filler[4];          // 0x1B -> 0x1B, 0x1C, 0x1D, 0x1F
};

#define _TMPSLOTNR      0x11
#define _SLOTMODES      0x19
#define _MODESETTINGS   0x1A
#define _CMD_REGISTER   0xF0

//These are the defaults for all settings
volatile registerLayout registerStack = {
  .sysStatus =                     0,     // 0x00
  .slotStatus =    {0,0,0,0,0,0,0,0},     // 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
  .address =    _I2C_DEFAULT_ADDRESS,     // 0x09
  .majorRelease =     _MAJOR_VERSION,     // 0x0A
  .minorRelease =     _MINOR_VERSION,     // 0x0B
  .debounceTime =                  5,     // 0x0C
  .midPressTime =                500,     // 0x0D 2 --> 0x0D 0x0E
  .longPressTime =              1500,     // 0x0F 2 --> 0x0F 0x10
  .tmpSlotNr =                     0,     // 0x11
  .tmpOutputFunc =                 0,     // 0x12
  .tmpPulseHighTime =              0,     // 0x13 2 --> 0x13 0x14
  .tmpPulseLowTime =               0,     // 0x15 2 --> 0x15 0x16
  .tmpStateDuration =              0,     // 0x17 2 --> 0x17 0x18
  .slotModes =                  0x00,     // 0x19
  .modeSettings =               0x00,     // 0x1A
  .filler =  {0xFF, 0xFF, 0xFF, 0xFF}     // 0x1B 4 --> 0x1B, 0x1C, 0x1D, 0x1F
};

//Cast 32bit address of the object registerStack with uint8_t so we can increment the pointer
uint8_t *registerPointer = (uint8_t *)&registerStack;

volatile byte registerNumber; 

static uint8_t    prevNextCode = 0;
static uint16_t   store=0;
uint32_t          interTimer, aliveTimer;

struct slotFields {
  byte      state;
  bool      isHigh;
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
  //Debug("set slot["); Debug(slotNr); Debug("] as OUTPUT ");
  switch(slotNr)
  {
    #if defined (__AVR_ATtiny841__) || defined (__AVR_ATtiny84__)
      case 0:   pinMode(10, OUTPUT); slot[slotNr].state = OUTPUT_DEFAULT; break;
      case 1:   pinMode( 9, OUTPUT); slot[slotNr].state = OUTPUT_DEFAULT; break;
      case 2:   pinMode( 8, OUTPUT); slot[slotNr].state = OUTPUT_DEFAULT; break;
      case 3:   pinMode( 7, OUTPUT); slot[slotNr].state = OUTPUT_DEFAULT; break;
      case 4:   pinMode( 3, OUTPUT); slot[slotNr].state = OUTPUT_DEFAULT; break;
      case 5:   pinMode( 2, OUTPUT); slot[slotNr].state = OUTPUT_DEFAULT; break;
      case 6:   pinMode( 1, OUTPUT); slot[slotNr].state = OUTPUT_DEFAULT; break;
      case 7:   pinMode( 0, OUTPUT); slot[slotNr].state = OUTPUT_DEFAULT; break;
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

} // setSlotAsOutput()

//==========================================================================
void setSlotAsInput(uint8_t slotNr)
{
  //Debug("set slot["); Debug(slotNr); Debug("] as INPUT  ");
  switch(slotNr)
  {
    #if defined (__AVR_ATtiny841__) || defined (__AVR_ATtiny84__)
      case 0:   pinMode(10, INPUT_PULLUP); break;
      case 1:   pinMode( 9, INPUT_PULLUP); break;
      case 2:   pinMode( 8, INPUT_PULLUP); break;
      case 3:   pinMode( 7, INPUT_PULLUP); break;
      case 4:   pinMode( 3, INPUT_PULLUP); break;
      case 5:   pinMode( 2, INPUT_PULLUP); break;
      case 6:   pinMode( 1, INPUT_PULLUP); break;
      case 7:   pinMode( 0, INPUT_PULLUP); break;
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
                if (newState) SETBIT(PORTB, 0); 
                else          CLEARBIT(PORTB, 0);
                break;
      case 1:   //--- PB1, slot1, D9
                if (newState) SETBIT(PORTB, 1); 
                else          CLEARBIT(PORTB, 1);
                break;
      case 2:   //--- PB2, slot2, D8
                if (newState) SETBIT(PORTB, 2); 
                else          CLEARBIT(PORTB, 2);
                break;
      case 3:   //--- PA7, slot3, D7
                if (newState) SETBIT(PORTA, 7); 
                else          CLEARBIT(PORTA, 7);
                break;
      case 4:   //--- PA3, slot4, D3
                if (newState) SETBIT(PORTA, 3); 
                else          CLEARBIT(PORTA, 3);
                break;
      case 5:   //--- PA2, slot5, D2
                if (newState) SETBIT(PORTA, 2); 
                else          CLEARBIT(PORTA, 2);
                break;
      case 6:   //--- PA1, slot6, D1
                if (newState) SETBIT(PORTA, 1); 
                else          CLEARBIT(PORTA, 1);
                break;
      case 7:   //--- PA0, slot7, D0
                if (newState) SETBIT(PORTA, 0); 
                else          CLEARBIT(PORTA, 0);
                break;
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
    #endif
  }
  slot[slotNr].state = OUTPUT_DEFAULT;
  
} // slotWrite()

//==========================================================================
void reBoot()
{
  Wire.end();

  //-- Pulse Output Slots (On) -
  for (int b=0; b<10; b++) 
  {
    for (uint8_t s=0; s<8; s++)
    {
      // only handle output slots
      if ( BIT_IS_HIGH(registerStack.slotModes, s) ) continue;
      slotWrite(s, HIGH);
      delay(100);
    }
    delay(1000);
    for (uint8_t s=0; s<8; s++)
    {
      // only handle output slots
      if ( BIT_IS_HIGH(registerStack.slotModes, s) ) continue;
      slotWrite(s, LOW);
      delay(100);
    }
  }

  //-- restart slave
  setup();

} //  reBoot()


//==========================================================================
void activatePulseOutput(uint8_t slotNr, uint16_t setHighTime
                                       , uint16_t setLowTime
                                       , uint16_t stateDuration)
{
  slot[slotNr].durationTimer = millis() + stateDuration;
  slot[slotNr].duration      = stateDuration;
  slot[slotNr].highTime      = setHighTime;
  slot[slotNr].highTimer     = millis() + setHighTime;
  slot[slotNr].lowTime       = setLowTime;
  slot[slotNr].lowTimer      = millis() + setLowTime;
  slot[slotNr].state         = OUTPUT_PULSE_ON;

  Debug("activatePulseOutput("); Debug(slotNr);  Debugln("):"); DebugFlash();
  Debugln(slot[slotNr].highTime); DebugFlash();
  Debugln(slot[slotNr].lowTime);  DebugFlash();
  Debugln(slot[slotNr].duration); DebugFlash();
  Debugln(slot[slotNr].state);    DebugFlash();

} // activatePulseOutput()


//==========================================================================
void activateToggleOutput(uint8_t slotNr, bool outputIsHigh, uint16_t stateDuration)
{

  Debug("activateToggleOutput("); Debug(slotNr); Debugln(")..");DebugFlash();
  slot[slotNr].durationTimer = millis() + stateDuration;
  slot[slotNr].duration      = stateDuration;
  slot[slotNr].isHigh        = outputIsHigh;
  slot[slotNr].highTime      = 0;
  slot[slotNr].highTimer     = 0;
  slot[slotNr].lowTime       = 0;
  slot[slotNr].lowTimer      = 0;
  slot[slotNr].state         = OUTPUT_DEFAULT;
  
} // activateToggleOutput()


//==========================================================================
void setOutput(uint8_t slotNr, byte outputFunc
                             , uint16_t pulseHighTime
                             , uint16_t pulseLowTime
                             , uint16_t stateDuration)
{
  bool outputIsHigh;
  uint8_t func = 0;

  Debug("outputFunc: "); Debug(outputFunc, BIN); DebugFlash();
  // (((p) & _BV(b)) != 0)
  /****
  if ( BIT_IS_HIGH(outputFunc, SLT_TOGGLE_BIT) ) 
  {
    func = 1;
    Debug(" ->function: 1 (toggle)"); 
  }
  else if ( BIT_IS_HIGH(outputFunc, SLT_PULSE_BIT) ) 
  {
    func = 2;
    Debug(" ->function: 2 (pulse)");
  }
  else 
  {
    Debug(" -> oeps... don't know :-( => ["); 
    Debug(outputFunc); 
    Debug("] ");
  }
  
  Debugln(); DebugFlash();
  ****/
  if ( BIT_IS_HIGH(outputFunc, SLT_TOGGLE_BIT) )   // Function is On/Off
  {
    Debug("activateToggleOutput() slot["); Debug(slotNr); Debug("]==> ");
    outputIsHigh = BIT_IS_HIGH(outputFunc, SLT_HIGH_BIT); // High or Low
    Debugln(outputIsHigh ? "HIGH" : "LOW");
    activateToggleOutput(slotNr, outputIsHigh, stateDuration);
  }
  else if( BIT_IS_HIGH(outputFunc, SLT_PULSE_BIT) )   // Function is Pulse
  {
    Debugln("activatePulseOutput() slot["); Debug(slotNr); Debugln("]");
    activatePulseOutput(slotNr, pulseHighTime, pulseLowTime, stateDuration);
  }

} // setOutput()


//==========================================================================
void updateSlotModes()
{
  Debug("slotMode --> ");
  showRegister(1, &registerStack.slotModes);
  Debugln();
  for (uint8_t slt=0; slt<8; slt++)
  {
    if ( BIT_IS_HIGH(registerStack.slotModes, slt) )   //-- input pin
    {
      setSlotAsInput(slt);
    }
    else
    {
      setSlotAsOutput(slt);
    }
  }
  Debug("slotMode ==> ");
  showRegister(1, &registerStack.slotModes);
  Debugln();
  
} // updateSlotModes()


//==========================================================================
void setup()
{
#if defined (__AVR_ATmega328P__)
  Serial.begin(230400);
  while(!Serial) {/* wait a bit */}
  Debugln("\n\rI2C_Lase_Tiny841_Slave startup ..\r\n");
#endif

  setSlotAsInput(_SW1);
  setSlotAsInput(_SW2);
  setSlotAsInput(_SW3);
  setSlotAsInput(_SW4);
  setSlotAsOutput(_LED1);
  setSlotAsOutput(_LED2);
  setSlotAsOutput(_LED3);
  setSlotAsOutput(_LED4);

  //---------------------------------------------------------------
  //-------------- now set Pulse Output -----------------------------
  //---------------------------------------------------------------
  //-- Pulse Output Slots (On) -
  for (int b=0; b<3; b++) 
  {
    for (uint8_t s=0; s<8; s++)
    {
      // only handle output slots
      if ( BIT_IS_HIGH(registerStack.slotModes, s) ) continue;
      slotWrite(s, HIGH);
      Debug("H");
      delay(100);
    }
    delay(100);
    for (uint8_t s=0; s<8; s++)
    {
      // only handle output slots
      if ( BIT_IS_HIGH(registerStack.slotModes, s) ) continue;
      slotWrite(s, LOW);
      Debug("L");
      delay(50);
    }
  }
  Debugln();
  
  //-->>> readConfig();
  
  //---------------------------------------------------------------
  //-------------- now set Pulse Output -----------------------------
  //---------------------------------------------------------------
  //-- Pulse Output Slots (On) -
  for (int b=0; b<3; b++) 
  {
    for (uint8_t s=0; s<8; s++)
    {
      // only handle output slots
      if ( BIT_IS_HIGH(registerStack.slotModes, s) ) continue;
      slotWrite(s, HIGH);
      Debug("H");
      delay(5);
    }
    delay(100);
    for (uint8_t s=0; s<8; s++)
    {
      // only handle output slots
      if ( BIT_IS_HIGH(registerStack.slotModes, s) ) continue;
      slotWrite(s, LOW);
      Debug("L");
      delay(5);
    }
    delay(100);
  }
  Debugln();
  
  startI2C();
  
  activatePulseOutput(_LED1, 300, 500, 0);
  activatePulseOutput(_LED2, 300, 100, 0);
  activatePulseOutput(_LED3, 300, 200, 0);
  activatePulseOutput(_LED4, 300, 300, 0);

  registerStack.sysStatus = 0;
  for(uint8_t r=0; r<7; r++)  registerStack.slotStatus[r] = 0;
  
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
  for (uint8_t s=0; s<8; s++)
  {
    Debug("Slot["); Debug(s);
    if ( BIT_IS_HIGH(registerStack.slotModes, s) )  
          {Debugln("] => input"); }
    else  {Debugln("] => output");}
  }

  Debugln("\n\rI2C_ExtPlus_Tiny841_Slave waiting for your command's ....\r\n");

#endif
  
} // setup()


//==========================================================================
void loop()
{
  /*
  * mail Loop takes about 142.85 micro seconds to complete with 
  * all eight slots serviced.
  */
  for (uint8_t s=0; s<8; s++)
  {
    //-- test if slot is set as input (default) 
    if (BIT_IS_HIGH(registerStack.slotModes, s) )  
    {
      //handleInput(s, slot[s]);
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
