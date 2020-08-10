/*
**    Program : I2Cstuff (part of I2C_ADW0720_Tiny841_Slave)
**
**    Copyright (c) 2020 Willem Aandewiel
**
**    TERMS OF USE: MIT License. See bottom of file.
***************************************************************************
*/


//------------------------------------------------------------------
void startI2C()
{
  Wire.end();

  Wire.begin(registerStack.address);

  // (Re)Declare the Events.
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

} // startI2C()


//------------------------------------------------------------------
boolean isConnected()
{
  Wire.beginTransmission((uint8_t)0);
  if (Wire.endTransmission() != 0) {
    return (false); // Master did not ACK
  }
  return (true);

} // isConnected()


//------------------------------------------------------------------
void normalizeSettings()
{
  byte modeReg = registerStack.modeSettings;

  if (registerStack.debounceTime > 250)     registerStack.debounceTime  =   250;
  if (registerStack.debounceTime <   5)     registerStack.debounceTime  =     5;
  if (registerStack.midPressTime  < 100)    registerStack.midPressTime  =   100;             
  if (registerStack.midPressTime  > 5000)   registerStack.midPressTime  =  5000;             
  if (registerStack.longPressTime < 300)    registerStack.longPressTime =   300;             
  if (registerStack.longPressTime > 10000)  registerStack.longPressTime = 10000;             
  if (registerStack.midPressTime  > registerStack.longPressTime)             
                    registerStack.midPressTime = 250; 
  if (registerStack.longPressTime < registerStack.midPressTime)             
                    registerStack.longPressTime = (2*registerStack.midPressTime);

} // normalizeSettings()


//------------------------------------------------------------------
void processCommand(byte command)
{
  if ((command & (1<<CMD_WRITECONF))) 
  {
    writeConfig();
  }
  if ((command & (1<<CMD_READCONF))) 
  {
    readConfig();
  }
  //-----> execute reBoot always last!! <-----
  if ((command & (1<<CMD_REBOOT))) 
  {
    reBoot();
  }

} // processCommand()

//------------------------------------------------------------------
//-- The master sends updated info that will be stored in the ------
//-- register(s)
//-- All Setters end up here ---------------------------------------
void receiveEvent(int numberOfBytesReceived)
{
  aliveTimer = millis();
  registerNumber = Wire.read(); //-- Get the memory map offset from the user

  if (registerNumber == _CMD_REGISTER)  //-- command
  {
    byte command = Wire.read(); //-- read the command
    processCommand(command);
    return;
  }
  if (registerNumber == _MODESETTINGS)  // mode Settings
  {
    normalizeSettings();
  }

  //-- Begin recording the following incoming bytes to the temp memory map
  //-- starting at the registerNumber (the first byte received)
  for (byte x = 0 ; x < numberOfBytesReceived - 1 ; x++) {
    uint8_t temp = Wire.read();
    if ( (x + registerNumber) < sizeof(registerLayout)) 
    {
      //Store the result into the register map
      if ( (registerNumber >= 0x01) && (registerNumber <= 0x0B) )
            { /* Can't update RO registers */ }
      else  registerPointer[registerNumber + x] = temp;
      if ( registerNumber == _SLOTMODES ) // a change to the slotModes is made
      {
        Debug("SLOTMODES!!>>:");
        showRegister(1, &registerStack.slotModes);
        Debugln(); DebugFlush();
        for (uint8_t slt=0; slt<8; slt++)
        {
          //-- if it's output change it!
          if ( BIT_IS_HIGH(registerStack.slotModes, slt) )   //-- output pin
          {
            setSlotAsInput(slt);
          }
          else    //-- change input pin
          {
            setSlotAsOutput(slt);
          }
        }
      } // _SLOTMODES
    }
  }

  if (registerNumber == _TMPSLOTNR)  // temp.slot Nr!!!
  { 
#if defined (__AVR_ATmega328P__)
    Debug("slot: "); Debug(registerStack.tmpSlotNr); DebugFlush();
    Debug("   set to:"); Debugln(registerStack.tmpOutputFunc, BIN); 
    Debug(" highTime:"); Debugln(registerStack.tmpPulseHighTime); 
    Debug("  lowTime:"); Debugln(registerStack.tmpPulseLowTime); 
    Debug(" Duration:"); Debugln(registerStack.tmpStateDuration); 
    DebugFlush();
#endif
    setOutputFunc(registerStack.tmpSlotNr, registerStack.tmpOutputFunc
                                     , registerStack.tmpPulseHighTime
                                     , registerStack.tmpPulseLowTime
                                     , registerStack.tmpStateDuration);
  }


  if (registerNumber == _MODESETTINGS)  // mode Settings
  {
    normalizeSettings();
  }

} //  receiveEvent()

//------------------------------------------------------------------
//-- The master aks's for the data from registerNumber onwards -----
//-- in the register(s) --------------------------------------------
//-- All getters get there data from here --------------------------
void requestEvent()
{
  aliveTimer = millis();

  //----- write max. 4 bytes starting at registerNumber -------
  for (uint8_t x = 0; ( (x < 4) &&(x + registerNumber) < (sizeof(registerLayout) - 1) ); x++) {
    Wire.write(registerPointer[(x + registerNumber)]);
  }
  //-- registerNumber == 0x03 => sysStatus
  //-- [7][6][5][4][3][2][1][0]  <--- sysStatus register bits
  //--  ^  ^  ^        ^  ^  ^
  //--  |  |  |        |  |  +-- <-- 1 if slot0 needs attention
  //--  |  |  |        |  +----- <-- 1 if slot1 needs attention
  //--  |  |  |        +-------- <-- 1 if slot2 needs attention
  //--  |  |  |  also for 3, 4 and 5
  //--  |  +-------------------- <-- 1 if slot6 needs attention
  //--  +----------------------- <-- 1 if slot7 needs attention
  //--
  //-- registerNumber == 0x04 => slotStatus[0]
  //-- [7][6][5][4][3][2][1][0]  <--- slotStatus[x] register bits
  //--              ^  ^  ^  ^ 
  //--              |  |  |  +--------------> 1 if Button pressed
  //--              |  |  +-----------------> 1 if Buttob Quick Released
  //--              |  +--------------------> 1 if Buttob Mid Released
  //--              +-----------------------> 1 if Buttob Long Released
  //--
  //-- registerNumber == 0x0B => slotStatus[7]
  if ( (registerNumber >= 0x04) && (registerNumber <= 0x0B) )
  {
    //--- clear sysStatus for this slot
    CLEARBIT(registerStack.sysStatus, (registerNumber - 4));
    #if defined (__AVR_ATmega328P__)
      Debug("CLEARBIT(sysStatus, "); Debug((registerNumber - 4)); Debugln(")");
      Debug("requestEvent(b): Reset byte[0x0"); Debug(registerNumber, HEX); Debugln("] ..");
    #endif
    registerPointer[registerNumber] = 0; // reset!
  }
  
} // requestEvent()

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
