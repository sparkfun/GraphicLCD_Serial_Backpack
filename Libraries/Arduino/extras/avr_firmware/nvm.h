/***************************************************************************
nvm.h

Function prototypes and memory map for the storage of parameters in
 non-volatile memory. 

02 May 2013 - Mike Hord, SparkFun Electronics

This code is released under the Creative Commons Attribution Share-Alike 3.0
 license. You are free to reuse, remix, or redistribute it as you see fit,
 so long as you provide attribution to SparkFun Electronics.

***************************************************************************/


#ifndef __nvm_h
#define __nvm_h

// EEPROM addresses for storing these settings.
#define SPLASH     0x00
#define REVERSE    0x01
#define BAUDRATE   0x02
#define BACKLIGHT  0x03

void    toggleSplash(void);
uint8_t getSplash(void);
void    toggleReverse(void);
uint8_t getReverse(void);
void    setBaudRate(char baudMode);
char    getBaudRate(void);
void    setBacklightLevel(uint8_t newLevel);
uint8_t getBacklightLevel(void);

#endif
