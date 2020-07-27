# Arduino library for the I2C_ExtPlus boards

<p>Find the project description at <a href="https://willem.aandewiel.nl/">www.aandewiel.nl</a> (not yet but soon).</p>
<br>

This library gives an interface between your own program and the I2C_LaSE board.

<center><img src="images/I2C_Extender_Top.png"></center>

To use it you have to include this library in your sketch

```
#include <#include <I2C_ExtPlus.h>
```

Declare an Encoder object (declare one for every I2C_ExtPlus board):

```
I2CEXTPL ExtenderBoard; // Create instance of the I2CEXTPL object
```

In the main <code>loop()</code> function handle <code>interruptPending</code>.

```
void loop() 
{
  byte statusReg, sysStatus;
  
  sysStatus = ExtenderBoard.getSysStatus();
  if (sysStatus == 0) return;
  
  if (bitRead(sysStatus, 1))
  {
    statusReg = ExtenderBoard.getslotStatus[4]();
    if (statusReg != 0) 
    {
      if (ExtenderBoard.isslot4Pressed() ) 
      {
        Serial.println(F("(slot4)-------> Button Pressed"));
      }
      if (ExtenderBoard.isslot4QuickReleased() ) 
      {
        Serial.println(F("(slot4)-------> Quick Release "));
        ExtenderBoard.setOutput(1, HIGH, 5000);
      }
      if (ExtenderBoard.isslot4MidReleased() ) 
      {
        Serial.println(F("(slot4)-------> Mid Release   "));
        ExtenderBoard.setOutputBlink(1, 250, 500, 0);
      }
      if (ExtenderBoard.isslot4LongReleased() ) 
      {
        Serial.println(F("(slot4)-------> Long Release  "));
        ExtenderBoard.setOutputBlink(1, 1000, 1700, 10000);
      }
    } // statusReg != 0
  }

}	// loop()
```

<center><img src="images/I2C_RotaryEncoder_v22-PCB_top.png"></center>

The library gives you the following setters:

| Setter             | Returns | Parms    | Description             |
|:-------------------|:-------:|:---------|:------------------------|
| setOutputToggle()  | bool    | uint8_t  | set the PWM value of the Red led (0 .. 255)|
| setOutputPulse     | bool    | uint8_t  | set the PWM value of the Green led (0 .. 255)|
| setDebounceTime()  | bool    | uint8_t  | set the Debounce Time of the switch (5 .. 250 micro seconds)|
| setMidPressTime()  | bool    | uint16_t | set the Mid Press Time of the switch (100 .. 5000 milli seconds)|
| setLongPressTime() | bool    | uint16_t | set the Long Press Time of the switch (300 .. 10000 milli seconds)|
| setI2Caddress()    | bool    | uint8_t  | set a new I2C address for this Slave (1 .. 127)|
| writeCommand()     | bool    | uint8_t  | write a command to the Slave (CMD_READCONF \| CMD_WRITECONF \| CMD_REBOOT)|

The library gives you the following getters:

| Getter             | Returns  | Parms | Description |
|:-------------------|:--------:|:-----:|:------------|
| isConnected()      | bool     | none  | returns true if connected, otherwise false
| getSysStatus()     | uint8_t  | none  | returns the status byte
| getSlotStatus()    | uint8_t  | uint8_t | returns the status of the selected slot
| getWhoAmI()        | int8_t   | none  | returns the Address Register
| getDebounceTime()  | uint8_t  | none  | returns the Debounce Time of the switch (5 .. 250 micro seconds)
| getMidPressTime()  | uint16_t | none  | returns the Mid Press Time of the switch (100 .. 5000 milli seconds)
| getLongPressTime() | uint16_t | none  | returns the Long Press Time of the switch (300 .. 10000 milli seconds)
| getModeSettings()  | uint8_t  | none  | returns the input/output mode settings for all the Slots
| getMajorRelease()  | uint8_t  | none  | returns the Major Firmware Release byte (0 .. 255)
| getMinorRelease()  | uint8_t  | none  | returns the Minor Firmware Release byte (0 .. 255)

And the library gives you the following helpers:

| Helper                 | Returns | Parms | Description |
|:-----------------------|:-------:|:-----:|:------------|
|isSlotPressed()          | bool    | uint8_t | true if the Button of slot is pressed
|isSlotQuickReleased()    | bool    | uint8_t | true if the Button is released before midPressTime
|isSlotMidReleased()      | bool    | uint8_t | true if the Button is released between midPressTime and longPressTime


<center><img src="images/I2CRE_Factory_Parts.png"></center>
