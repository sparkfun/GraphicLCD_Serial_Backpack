#include <avr/io.h>
#include "serial.h"

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

void putLine(uint8_t *TXData)
{
	while (*TXData != '\0')
	{
		putChar(*(TXData++));
	}
	putChar('\n');
}
