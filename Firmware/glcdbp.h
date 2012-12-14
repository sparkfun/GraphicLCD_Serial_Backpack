#ifndef __glcdbp_h
#define __glcdbp_h

//#define F_CPU 	16000000

//128x64 definitions
#define EN 			0	//PC0
#define RS 			1	//PC1, D_I?
#define R_W 		2	//PC2
#define RESET 		3	//PC3
#define CS1 		4	//PC4
#define CS2 		5	//PC5

#define nBL_EN 		2	//PB2 is backlight enable, and is active low

#define BPS 0
#define BACKLIGHT 1
#define SPLASH 2
#define REV 3

const int BACKLIGHT_VALUE 	= 0;

typedef enum DISPLAY_TYPE {SMALL, LARGE} DISPLAY_TYPE;

void ioInit(void);
void timerInit(void);

#endif