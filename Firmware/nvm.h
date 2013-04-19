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
