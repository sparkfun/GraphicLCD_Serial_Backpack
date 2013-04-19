#ifndef __io_support_h
#define __io_support_h

#define nBL_EN 		2	//PB2 is backlight enable, and is active low
/*  I think I can be rid of these things- I had intended them to be EEPROM
locations, but forgot they were here when I created the nvm.h file.
#define BPS 0
#define BACKLIGHT 1
#define SPLASH 2
#define REV 3
*/
#define EN 			0	// PC0
#define RS 			1	// PC1
#define R_W 		2	// PC2
#define RESET 		3	// PC3
#define CS1 		4	// PC4
#define CS2 		5	// PC5

void 		ioInit(void);
void 		setData(uint8_t data);
uint8_t 	readData(void);
void 		LATrigger(void);

#endif
