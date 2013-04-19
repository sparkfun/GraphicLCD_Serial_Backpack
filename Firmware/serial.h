#ifndef __serial_h
#define __serial_h

enum {
	BR4800		= 416,
	BR9600  	= 207,
	BR19200		= 103,
	BR38400		= 51,
	BR57600		= 34,
	BR115200	= 16
	};

void serialInit(uint16_t baudRate);
void putChar(uint8_t TXData);
void putHex(uint8_t TXData);
void putDec(uint8_t TXData);
void putBin(uint8_t TXData);
void putLine(uint8_t *TXData);
char serialBufferPop(void);
void clearBuffer(void);

#endif
