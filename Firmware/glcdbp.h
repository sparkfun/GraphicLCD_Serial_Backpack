#ifndef __glcdbp_h
#define __glcdbp_h

#define BUF_DEPTH 256  // Ring buffer size.

typedef enum DISPLAY_TYPE {SMALL, LARGE} DISPLAY_TYPE;
typedef enum PIX_VAL {ON, OFF} PIX_VAL;

void ioInit(void);
void timerInit(void);

#endif
