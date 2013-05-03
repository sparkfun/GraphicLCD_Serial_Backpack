/***************************************************************************
nvm.c

Non-volatile memory file for the serial graphical LCD backpack project.
 This file provides the functions needed to set and get the sticky settings
 from EEPROM.

02 May 2013 - Mike Hord, SparkFun Electronics

This code is released under the Creative Commons Attribution Share-Alike 3.0
 license. You are free to reuse, remix, or redistribute it as you see fit,
 so long as you provide attribution to SparkFun Electronics.

***************************************************************************/

#include <avr/eeprom.h>
#include "nvm.h"

// Important note: on factory/post-programming reset, all EEPROM locations
//  initialize to 0xff. I didn't want to put in some kind of "first boot"
//  key, so there's some logic to handle, as transparently as possible, the
//  fact of the default values.

// Toggle the splash display mode. Default is ON. Only the LSb of the byte
//  is actually checked. Since default mode is ON, and that equates to a '1'
//  in the LSb, we're okay just reading and writing things as they are.
void toggleSplash(void)
{
  uint8_t temp = eeprom_read_byte((const uint8_t *)SPLASH);
  temp ^= 0x01;
  eeprom_write_byte((uint8_t *)SPLASH, temp);
}

uint8_t getSplash(void)
{
  return eeprom_read_byte((const uint8_t *)SPLASH);
}

// Toggle reverse mode (that is to say, whether the background of the display
//  consists of pixels that are "on" (reverse == 1) or "off" (reverse == 0).
//  Default should be off; that's what we're *used* to seeing. To achieve that,
//  the value stored in bit 0 of the EEPROM location is the complement of the
//  actual value, i.e., a '0' in the LSb indicates that reverse mode will be
//  enabled on power up, and vice versa.
void toggleReverse(void)
{
  uint8_t temp = 0x01 & eeprom_read_byte((const uint8_t *)REVERSE);
  temp ^= 0x01;
  eeprom_write_byte((uint8_t *)REVERSE, temp);
}

// By complementing this before we return it, we ensure that the default state
//  of 0xff corresponds to reverse mode OFF.
uint8_t getReverse(void)
{
  return ~eeprom_read_byte((const uint8_t *)REVERSE);
}

// We don't want the user to set an invalid baud rate, so we don't allow it.
void setBaudRate(char baudMode)
{
  if (baudMode >= '1' && baudMode <= '6')
    eeprom_write_byte((uint8_t *)BAUDRATE, baudMode);
}

// We check the baud rate when we read it, anyhow, and if the value stored
//  in EEPROM isn't valid, we default to 115200 baud.
char getBaudRate(void)
{
  return eeprom_read_byte((const uint8_t *)BAUDRATE);
}

// Anything greater than 100 defaults to 100, which is a percentage. Since we
//  don't care what it is, we don't have to check to make sure the value is
//  actually valid.
void setBacklightLevel(uint8_t newLevel)
{
  eeprom_write_byte((uint8_t *)BACKLIGHT, newLevel);
}

// Likewise, we don't have to check to see if this value is valid, either.
uint8_t getBacklightLevel(void)
{
  return eeprom_read_byte((const uint8_t *)BACKLIGHT);
}
