/***************************************************************************
serial.h

Serial support- function prototypes and constant defines for baud rates.

02 May 2013 - Mike Hord, SparkFun Electronics

This code is released under the Creative Commons Attribution Share-Alike 3.0
 license. You are free to reuse, remix, or redistribute it as you see fit,
 so long as you provide attribution to SparkFun Electronics.

***************************************************************************/

#ifndef __serial_h
#define __serial_h

// These are the values for the baud rate generator corresponding to these bit
//  rates. For more information about bit error percentages and what these
//  numbers mean, see the datasheet for the processor.
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
void putLine(char *TXData);
char serialBufferPop(void);
void clearBuffer(void);

#endif
