#include <avr/io.h>
#include "serial.h"
#include "glcdbp.h"

// These variables are defined in glcdbp.c, and are used for the input buffer
//   from the serial port. We need to be able to access them here because we'll
//   want to abstract popping from the buffer to a function.
extern volatile uint8_t 	rxRingBuffer[BUF_DEPTH];
extern volatile uint16_t 	rxRingHead;
extern volatile uint16_t	rxRingTail;
extern volatile uint16_t	bufferSize;

// Initialize the serial port hardware.
void serialInit(uint16_t baudRate)
{
	// Set baud rate 
	UBRR0 = baudRate;

	// Enable receiver and transmitter 
	UCSR0A = (1<<U2X0);
	UCSR0B = (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0);	//Enable Interrupts on receive

	UCSR0C = (1<<UCSZ00)|(1<<UCSZ01);
}

void putChar(uint8_t TXData)
{
	while ( !( UCSR0A & (1<<UDRE0)) );	// Wait for empty transmit buffer
	UDR0 = TXData;						// Put data into buffer; uC automatically
										//  sends data once it's written.
}

void putHex(uint8_t TXData)
{
	uint8_t hexChars[2];				// We'll use an array to buffer the
	// First, let's isolate the two halves of the byte in question.
	hexChars[0] = (0x0F) & TXData;
	hexChars[1] = ((0xF0) & TXData)>>4;
	// Okay, now we can convert them to an ASCII value.
	if (hexChars[1] < 10) hexChars[1] += '0';
	else hexChars[1] += ('A'-10);
	putChar(hexChars[1]);
	if (hexChars[0] < 10) hexChars[0] += '0';
	else hexChars[0] += ('A'-10);
	putChar(hexChars[0]);
}

void putDec(uint8_t TXData)
{
	uint8_t ones = TXData % 10;
	TXData -= ones;
	uint8_t tens = (TXData % 100)/10;
	uint8_t huns = TXData / 100;
	putChar((huns) + '0');
	putChar((tens) + '0');
	putChar((ones) + '0');
}

void putBin(uint8_t TXData)
{
	for (char i = 0; i < 8; i++)
	{
		if ((TXData & 0x01) == 0x01) putChar('1');
		else putChar('0');
		TXData = TXData>>1;
	}
}

void putLine(char *TXData)
{
	while (*TXData != '\0')
	{
		putChar(*(TXData++));
	}
	putChar('\n');
	putChar('\r');
}

char serialBufferPop(void)
{
  bufferSize--;
  char retVal = rxRingBuffer[rxRingTail++];
  if (rxRingTail == BUF_DEPTH) rxRingTail = 0;
  return retVal;
}

void clearBuffer(void)
{
  bufferSize = 0;
  rxRingTail = 0;
  rxRingHead = 0;
}
