#include <avr/eeprom.h>
#include "nvm.h"

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

void toggleReverse(void)
{
  uint8_t temp = 0x01 & eeprom_read_byte((const uint8_t *)REVERSE);
  temp ^= 0x01;
  eeprom_write_byte((uint8_t *)REVERSE, temp);
}

uint8_t getReverse(void)
{
  return ~eeprom_read_byte((const uint8_t *)REVERSE);
}

void setBaudRate(char baudMode)
{
  if (baudMode >= '1' && baudMode <= '6')
    eeprom_write_byte((uint8_t *)BAUDRATE, baudMode);
}

char getBaudRate(void)
{
  return eeprom_read_byte((const uint8_t *)BAUDRATE);
}

void setBacklightLevel(uint8_t newLevel)
{
  eeprom_write_byte((uint8_t *)BACKLIGHT, newLevel);
}

uint8_t getBacklightLevel(void)
{
  return eeprom_read_byte((const uint8_t *)BACKLIGHT);
}
