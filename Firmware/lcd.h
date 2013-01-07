#ifndef __lcd_h
#define __lcd_h

void		lcdConfig(void);
void 		lcdSetData(uint8_t data);
uint8_t 	lcdReadData(void);
void 		lcdDrawPixel(uint8_t x, uint8_t y, PIX_VAL pixel);
void 		lcdDrawLine(uint8_t p1x, uint8_t p1y, uint8_t p2x, uint8_t p2y);

#endif
