#include <avr/io.h>
#include "serial.h"

// Initialize the serial port hardware.
void serialInit(uint16_t baudRate)
{
	// Set baud rate 
	UBRR0 = baudRate;

	// Enable receiver and transmitter 
	UCSR0A = (1<<U2X0);
	UCSR0B = (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0);	//Enable Interrupts on receive character

	UCSR0C = (1<<UCSZ00)|(1<<UCSZ01);
}

void putChar(uint8_t TXData)
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) );
	/* Put data into buffer, sends the data */
	UDR0 = TXData;
}