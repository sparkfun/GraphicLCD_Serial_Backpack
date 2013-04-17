#ifndef __lcd_h
#define __lcd_h

void		lcdConfig(void);
void		lcdClearScreen(void);
void 		lcdDrawPixel(uint8_t x, uint8_t y, PIX_VAL pixel);
void 		lcdDrawLine(int8_t p1x, int8_t p1y, int8_t p2x, int8_t p2y);
void 		lcdDrawCircle(uint8_t x0, uint8_t y0, uint8_t r);
void		lcdDrawBox(int8_t p1x, int8_t p1y, int8_t p2x, int8_t p2y);

#endif
