/***************************************************************************
t6963.h

t6963 controller header file. Function prototypes and constant definitions.

02 May 2013 - Mike Hord, SparkFun Electronics

This code is released under the Creative Commons Attribution Share-Alike 3.0
 license. You are free to reuse, remix, or redistribute it as you see fit,
 so long as you provide attribution to SparkFun Electronics.

***************************************************************************/


#ifndef __t6963_h
#define __t6963_h

// Defines for the BitSR function. Bit 3 of the command corresponds to SET
//  make pixel light) or RESET (make pixel dark) the bit referred to by bits
//  2:0; to make life a little easier, I defined these.
#define PIX_DK 0x00
#define PIX_LT 0x08

void     t6963WriteData(uint8_t data);
uint8_t  t6963ReadData(void);
void     t6963WriteCmd(uint8_t command);
uint8_t  t6963ReadStatus(void);
void     t6963DisplayInit(void);
void     t6963SetPointer(uint8_t x, uint8_t y);
void     t6963BusyWait(void);
void     t6963Clear(void);
void     t6963DrawPixel(uint8_t x, uint8_t y, PIX_VAL pixel);
void     t6963ReadBlock(uint8_t x, uint8_t y, uint8_t *buffer);
void     t6963BitSR(uint8_t bit, uint8_t SR);

#endif

/* 
The T6963 operates in a significantly different way to the ks0108b. The
ks0108b system is a "pure" graphical LCD- it has one memory location per pixel,
breaks up its memory space into 8-pixel columns, and that's it.

The T6963 is a much more advanced system. It has a much larger memory space,
allowing for multiple displays to be stored and all-at-once display flips to
occur by simply changing a pointer. It has built in text/character generation.
We're going to ignore basically all the advanced features, and concentrate on
text mode only. That's kind of a bummer, because the advance features are cool,
but doing things this way keeps the code easy.

Finally, rather than rows of columns 8 pixels high, the T6963 is broken up into
rows one pixel high and of a width defined by the user. Within that row, each
byte is individually addressable. It also has a nice one-bit-only write feature
which makes the drawPixel command more slick than it is for the ks0108b.
*/
