#!/bin/bash
#
./avrdude/bin/avrdude -C./avrdude/avrdude.conf -v -pattiny841 -cusbtiny -e -Uefuse:w:0b11111110:m -Uhfuse:w:0b11010110:m -Ulfuse:w:0xE2:m -Uflash:w:bootloaders/empty_all.hex:i
#
#
./avrdude/bin/avrdude -C./avrdude/avrdude.conf -v -pattiny841 -cusbtiny -Uflash:w:./arduinoBuild/I2C_ADW0720_Tiny841_Slave.ino.hex:i
#

