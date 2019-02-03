# Endurance_EEPROM
a try implement an endurance way to save log data in eeprom, going to use it for 24LC64 
i have no experience with EEPROM but this is how i attmept to make a library that should have wear leveling and safe to write logging into eeprom. 

its not file system so just page writing data with pointer of the data.. :)

i think a good library is written by [nasa EEFS](https://github.com/nasa/eefs)?
but i think its not for what im planning for...

## Links to read:

1- [AVR101: High Endurance EEPROM Storage](http://ww1.microchip.com/downloads/en/AppNotes/doc2526.pdf)
2- [EEPROM Reliability and Wear Leveling](http://www.mosaic-industries.com/embedded-systems/sbc-single-board-computers/freescale-hcs12-9s12-c-language/instrument-control/eeprom-lifetime-reliability-wear-leveling)




### [24LC64](http://ww1.microchip.com/downloads/en/DeviceDoc/21189T.pdf) specs:
*  64Kbit = 8Kbyte = 250 Write Page
*  Page Write Time 5 ms, max.
*  32-Byte Page Write Buffer
*  More than 1 Million Erase/Write Cycles
*  I2C address: 0x50
