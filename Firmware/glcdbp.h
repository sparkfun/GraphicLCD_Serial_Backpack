#ifndef __glcdbp_h
#define __glcdbp_h

#define nBL_EN 		2	//PB2 is backlight enable, and is active low

#define BPS 0
#define BACKLIGHT 1
#define SPLASH 2
#define REV 3

typedef enum DISPLAY_TYPE {SMALL, LARGE} DISPLAY_TYPE;

void ioInit(void);
void timerInit(void);
void LATrigger(void);

#endif