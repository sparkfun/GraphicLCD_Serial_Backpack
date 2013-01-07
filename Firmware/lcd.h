#ifndef __ks0108b_h
#define __ks0108b_h

void 		lcdSetData(uint8_t data);
uint8_t 	lcdReadData(void);
void 		lcdDrawPixel(uint8_t x, uint8_t y, PIX_VAL pixel);

#endif