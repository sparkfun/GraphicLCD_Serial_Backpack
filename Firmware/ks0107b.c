#include <avr/io.h>
#include "ks0107b.h"

// Write a data byte to the controller. Data lines 7:2 are connected to pins
//	B7:2 and 1:0 are on D1:0.
void ks0107bWriteData(uint8_t data)
{
	PORTB |= data & 0xFC;			// Mask off PB1:0 so we don't change them
									//  and then write the other 6 bits.
	PORTD |= data & 0x03;			// Mask off PD7:2 so we don't change them
									//	and then write the other two bits.
									//  The data is now in place.
}

uint8_t ks0107bReadData(void)
{	
	uint8_t data;
	data = PINB & 0xFC;				// Mask off PB1:0 so we don't change them
									//  and then write the other 6 bits.
	data |= PIND & 0x03;			// Mask off PD7:2 so we don't change them
									//	and then write the other two bits.
									//  The data is now in place.
}

void ks0107bWriteCmd(uint8_t command)
{	
	PORTB |= command & 0xFC;		// Mask off PB1:0 so we don't change them
									//  and then write the other 6 bits.
	PORTD |= command & 0x03;		// Mask off PD7:2 so we don't change them
									//	and then write the other two bits.
									//  The data is now in place.
}

uint8_t ks0107bReadStatus(void)
{	
	uint8_t status;
	status = PINB & 0xFC;				// Mask off PB1:0 so we don't change them
									//  and then write the other 6 bits.
	status |= PIND & 0x03;			// Mask off PD7:2 so we don't change them
									//	and then write the other two bits.
									//  The data is now in place.
}
