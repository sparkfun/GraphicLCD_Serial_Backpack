#ifndef __io_support_h
#define __io_support_h

#define nBL_EN 		2	//PB2 is backlight enable, and is active low

// Pins for the ks0108b (128x64) display
#define EN      0	// PC0
#define RS      1	// PC1
#define R_W     2	// PC2
#define RESET   3	// PC3
#define CS1     4	// PC4
#define CS2     5	// PC5

// Pins for the t6963 (160x128) display
#define WR      0	// PC0
#define RD      1	// PC1
#define CE      2	// PC2
#define CD      3	// PC3
#define HALT    4	// PC4
#define RST     5	// PC5

void    ioInit(void);
void    setData(uint8_t data);
uint8_t readData(void);

#endif
