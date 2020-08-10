/*
**    Program : fsmInputs (part of I2C_ADW0720_Tiny841_Slave)
**
**    Copyright (c) 2020 Willem Aandewiel
**
**    TERMS OF USE: MIT License. See bottom of file.
***************************************************************************
*/



//==========================================================================
void handleInput(uint8_t slotNr)
{
  //---------------------------------------------------------------
  //-------------- finite state button ----------------------------
  //---------------------------------------------------------------
#if defined ( __AVR_ATmega328P__)
  if (registerStack.sysStatus != 0)
  {
    //Debug("[-stat7-- -stat6-- -stat5-- -stat4-- -stat3-- -stat2-- -stat1-- -stat0-- ");
    //Debugln("sysStat-]");
    Debug("handleInput() -> sysStat: ");
    showRegister(1, &registerStack.sysStatus);
    Debug(" slotStat["); Debug(slotNr); Debug("]: ");
    showRegister(1, &registerStack.slotStatus[slotNr]);
    Debugln();
  }
#endif
  switch(slot[slotNr].state) 
  {
  case BTN_INIT:
    if ( !slotRead(slotNr) )   // Switches are Active LOW
    {
      slot[slotNr].bounceTimer      = micros();
      registerStack.slotStatus[slotNr] = 0;
      slot[slotNr].state            = BTN_FIRST_PRESS;
      slot[slotNr].pressedStateSend = false;
    }
    break;

  case BTN_FIRST_PRESS:
    if ((micros() - slot[slotNr].bounceTimer) > registerStack.debounceTime) 
    {
      //-- check if slot still active (LOW) after debounce time
      if ( !slotRead(slotNr) ) 
      {
        slot[slotNr].state         = BTN_IS_PRESSED;
        slot[slotNr].durationTimer = millis();
      } 
      else 
      {
        slot[slotNr].state    = BTN_INIT;
      }
    }
    break;

  case BTN_IS_PRESSED:
    //-- check still active (LOW)
    if ( !slotRead(slotNr) ) 
    {
      if ((!slot[slotNr].pressedStateSend) && BIT_IS_LOW(registerStack.slotStatus[slotNr], SLT_PRESSED_BIT) ) // not already set?
      {
        SETBIT(registerStack.slotStatus[slotNr], SLT_PRESSED_BIT);  // button pressed
        SETBIT(registerStack.sysStatus, slotNr); // set sysStatus for this slot
        slot[slotNr].pressedStateSend = true;
        #if defined (__AVR_ATmega328P__)
          Debug("fsmSwitches() ["); Debug(slotNr); Debugln("] --> Pressed");
        #endif
      }
    } 
    else   //-- button is released (HIGH)
    {
      slot[slotNr].bounceTimer = micros();
      slot[slotNr].state       = BTN_FIRST_RELEASE;
    }
    break;

  case BTN_FIRST_RELEASE:
    if ((micros() - slot[slotNr].bounceTimer) > registerStack.debounceTime) 
    {
      //-- check if button is released (HIGH)
      if (  slotRead(slotNr) )
      {
        slot[slotNr].state    = BTN_IS_RELEASED;
      } 
      else 
      {
        slot[slotNr].state    = BTN_IS_PRESSED;
      }
    }
    break;

  case BTN_IS_RELEASED:
    //aliveTimer = millis();
    if ((millis() - slot[slotNr].durationTimer) > registerStack.longPressTime) 
    {
      if (BIT_IS_LOW(registerStack.slotStatus[slotNr], SLT_LONGRELEASE_BIT) )  // not already set?
      {
        SETBIT(registerStack.slotStatus[slotNr], SLT_LONGRELEASE_BIT);  // release Long
        SETBIT(registerStack.sysStatus, slotNr);
        slot[slotNr].pressedStateSend = false;
        #if defined (__AVR_ATmega328P__)
          Debug("fsmSwitches() ["); Debug(slotNr); Debugln("] --> Long Released");
        #endif
      }
    } else if ((millis() - slot[slotNr].durationTimer) > registerStack.midPressTime) {
      if (BIT_IS_LOW(registerStack.slotStatus[slotNr], SLT_MIDRELEASE_BIT) ) // not already set?
      {
        SETBIT(registerStack.slotStatus[slotNr], SLT_MIDRELEASE_BIT);  // release Mid
        SETBIT(registerStack.sysStatus, slotNr);
        slot[slotNr].pressedStateSend = false;
        #if defined (__AVR_ATmega328P__)
          Debug("fsmSwitches() ["); Debug(slotNr); Debugln("] --> Mid Released");
        #endif
      }
    } else {
      if (BIT_IS_LOW(registerStack.slotStatus[slotNr], SLT_QUICKRELEASE_BIT) ) // already set?
      {
        SETBIT(registerStack.slotStatus[slotNr], SLT_QUICKRELEASE_BIT);  // release Quick
        SETBIT(registerStack.sysStatus, slotNr);
        slot[slotNr].pressedStateSend = false;
        #if defined (__AVR_ATmega328P__)
          Debug("fsmSwitches() ["); Debug(slotNr); Debugln("] --> Quick Released");
        #endif
      }
    }
    slot[slotNr].state       = BTN_INIT;
    slot[slotNr].bounceTimer = 0;
    break;

  default:
    slot[slotNr].state       = BTN_INIT;

  } // switch()

} // handleInput()

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
