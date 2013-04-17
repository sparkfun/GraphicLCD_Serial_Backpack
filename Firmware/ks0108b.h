#ifndef __ks0108b_h
#define __ks0108b_h

typedef enum SCREEN_SIDE {LEFT, RIGHT, BOTH} SCREEN_SIDE;

void 		ks0108bWriteData(uint8_t data);
void 		ks0108bReadBlock(uint8_t address, uint8_t blockSize, uint8_t *buffer);
void 		ks0108bReadSet(uint8_t address);
uint8_t 	ks0108bReadData(uint8_t x);
void 		ks0108bSetColumn(uint8_t address);
void 		ks0108bSetPage(uint8_t address);
uint8_t 	ks0108bReadStatus(void);
void		ks0108bDisplayOn(void);
void 		ks0108bReset(void);
void 		ks0108bBusyWait(void);
void		ks0108bSetStartLine(void);
void		strobeEN(void);
void 		ks0108bClear(void);
void 		ks0108bDrawPixel(uint8_t x, uint8_t y, PIX_VAL pixel);
void		ks0108bDrawColumn(uint8_t x, uint8_t y, uint8_t colVal);
void		delay(void);

#endif

/* All about the ks0108b/ks0107b system
The datasheet isn't great, nor are other explanations I've found, so
here's my little addition to the muddle:

A 128x64 display is, in fact, treated as TWO 64x64 displays. Each display
consists of 8 pages, and each page is 8 pixels high by 64 pixels wide.

Thus, if you set the X address to 0, the Y address to zero, and then write
b01010101 as display data, you can expect to get a pattern of alternating
off/on pixels in the first column in the upper left hand corner. The column
counter will automatically increment, and you can write a different pattern
to the next column. You may repeat this until you've written all 64 columns
across on the page, then either switch to the second "display" and repeat OR
switch to the next page and repeat; the datasheet is unclear as to what
happens if the user fails to intervene at the end of a series of Y addresses.

It's possible to read back the data describing a column; thus, a user can
perform read-modify-write operations without having to mirror the entire
memory space in the MCU.

Finally, I'll point out that this is a VERY barebones LCD- there are no onboard
provisions for text, image rendering, vector plotting, or anything else. ALL of
that *must* be calculated and then raster-rendered by the host MCU.
*/
