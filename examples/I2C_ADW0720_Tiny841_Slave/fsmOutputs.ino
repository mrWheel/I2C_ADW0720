/*
**    Program : fsmOutputs (part of I2C_ADW0720_Tiny841_Slave)
**
**    Copyright (c) 2020 Willem Aandewiel
**
**    TERMS OF USE: MIT License. See bottom of file.
***************************************************************************
*/


//==========================================================================
//void handleOutput(uint8_t slotNr, slotFields &slot)
void handleOutput(uint8_t slotNr)
{
  //---------------------------------------------------------------
  //-------------- finite state button ----------------------------
  //---------------------------------------------------------------
  
  switch(slot[slotNr].state) 
  {
    case OUTPUT_DEFAULT:
              slotWrite(slotNr, slot[slotNr].isHigh);

              if (slot[slotNr].duration > 0) 
              {
                if (millis() > slot[slotNr].durationTimer)
                {
                  registerStack.slotStatus[slotNr] = 0;
                  SETBIT(registerStack.slotStatus[slotNr], SLT_TOGGLE_BIT);
                  slot[slotNr].duration      = 0;
                  slot[slotNr].durationTimer = 0;
                  slot[slotNr].isHigh        = false;
                  slot[slotNr].pwmValue      = 0;
//                slot[slotNr].state         = OUTPUT_DEFAULT;
//                break;
                }
              }
              slot[slotNr].state = OUTPUT_DEFAULT;
              break;
              
    case OUTPUT_PULSE_ON:
              //Debug("slot["); Debug(slotNr); Debugln("] PULSE_ON");
              //Debug("highTime : "); Debugln(slot[slotNr].highTime); DebugFlush();
              //Debug("lowTime  : "); Debugln(slot[slotNr].lowTime);  DebugFlush();
              //Debug("duration : "); Debugln(slot[slotNr].duration); DebugFlush();
              if (slot[slotNr].duration > 0) 
              {
                //Debug("millis() : "); Debugln( millis() ); DebugFlush();
                //Debug("timer    : "); Debugln( slot[slotNr].durationTimer ); DebugFlush();
                if (millis() > slot[slotNr].durationTimer)
                {
                  registerStack.slotStatus[slotNr] = 0;
                  SETBIT(registerStack.slotStatus[slotNr],   SLT_TOGGLE_BIT);
                  slotWrite(slotNr, LOW);
                  Debugln("duration elapsed");
                  slot[slotNr].duration      = 0;
                  slot[slotNr].durationTimer = 0;
                  slot[slotNr].isHigh        = false;
                  slot[slotNr].pwmValue      = 0;
                  slot[slotNr].state         = OUTPUT_DEFAULT;
                  break;
                }
              }
              slotWrite(slotNr, HIGH);
              //Debug("highTimer  : "); Debugln( slot[slotNr].highTimer ); DebugFlush();
              if (millis() > slot[slotNr].highTimer)
              {
                slot[slotNr].lowTimer = millis() + slot[slotNr].lowTime;
                slot[slotNr].state = OUTPUT_PULSE_OFF;
                break;
              }
              slot[slotNr].state = OUTPUT_PULSE_ON;
              break;
              
    case OUTPUT_PULSE_OFF:
              //Debug("slot["); Debug(slotNr); Debugln("] PULSE_OFF");
              slotWrite(slotNr, LOW);
              if (millis() > slot[slotNr].lowTimer)
              {
                slot[slotNr].highTimer = millis() + slot[slotNr].highTime;
                slot[slotNr].isHigh    = true;
                slot[slotNr].pwmValue  = 0;
                slot[slotNr].state     = OUTPUT_PULSE_ON;
                break;
              }
              slot[slotNr].state     = OUTPUT_PULSE_OFF;
              break;
              
    case OUTPUT_PWM_SET:
              Debug("slot["); Debug(slotNr); Debugln("] PWM_SET");
              Debug("pwmValue : "); Debugln(slot[slotNr].pwmValue); DebugFlush();
              Debug("duration : "); Debugln(slot[slotNr].duration); DebugFlush();
              if (slot[slotNr].pwmValue == 0)
              {
                Debugln("OUTPUT_PWM_SET: switched off by value 0");
                registerStack.slotStatus[slotNr] = 0;
                SETBIT(registerStack.slotStatus[slotNr], SLT_TOGGLE_BIT);
                slotWrite(slotNr, LOW);
                slot[slotNr].isHigh = false;
                slot[slotNr].state  = OUTPUT_DEFAULT;
                break;
              }
              if (slot[slotNr].duration > 0) 
              {
                //Debug("millis() : "); Debugln( millis() ); DebugFlush();
                //Debug("timer    : "); Debugln( slot[slotNr].durationTimer ); DebugFlush();
                if (millis() > slot[slotNr].durationTimer)
                {
                  Debugln("duration elapsed");
                  registerStack.slotStatus[slotNr] = 0;
                  SETBIT(registerStack.slotStatus[slotNr], SLT_TOGGLE_BIT);
                  slotWrite(slotNr, LOW);
                  slot[slotNr].duration      = 0;
                  slot[slotNr].durationTimer = 0;
                  slot[slotNr].isHigh        = false;
                  slot[slotNr].pwmValue      = 0;
                  slot[slotNr].state         = OUTPUT_DEFAULT;
                  break;
                }
              }
              slotWritePWM(slotNr, slot[slotNr].pwmValue);
              slot[slotNr].state = OUTPUT_PWM;
              break;
              
    case OUTPUT_PWM:
              //Debug("slot["); Debug(slotNr); Debugln("] PWM");
              //Debug("highTime : "); Debugln(slot[slotNr].highTime); DebugFlush();
              //Debug("duration : "); Debugln(slot[slotNr].duration); DebugFlush();
              if (slot[slotNr].pwmValue == 0)
              {
                Debugln("OUTPUT_PWM: switched off by value 0");
                registerStack.slotStatus[slotNr] = 0;
                SETBIT(registerStack.slotStatus[slotNr], SLT_TOGGLE_BIT);
                slotWrite(slotNr, LOW);
                slot[slotNr].pwmValue = 0;
                slot[slotNr].state    = OUTPUT_DEFAULT;
                break;
              }
              if (slot[slotNr].duration > 0) 
              {
                //Debug("millis() : "); Debugln( millis() ); DebugFlush();
                //Debug("timer    : "); Debugln( slot[slotNr].durationTimer ); DebugFlush();
                if (millis() > slot[slotNr].durationTimer)
                {
                  Debugln("duration elapsed");
                  registerStack.slotStatus[slotNr] = 0;
                  SETBIT(registerStack.slotStatus[slotNr], SLT_TOGGLE_BIT);
                  slotWrite(slotNr, LOW);
                  slot[slotNr].duration      = 0;
                  slot[slotNr].durationTimer = 0;
                  slot[slotNr].isHigh        = false;
                  slot[slotNr].pwmValue      = 0;
                  slot[slotNr].state         = OUTPUT_DEFAULT;
                  break;
                }
              }
              slot[slotNr].state = OUTPUT_PWM;
              break;
              
    default: slot[slotNr].state = OUTPUT_DEFAULT;

  } // switch(slot[slotNr].state)
  
} //  handleOutput()

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
