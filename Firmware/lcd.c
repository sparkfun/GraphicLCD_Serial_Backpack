#include<avr/io.h>
#include "lcd.h"

void lcdSetData(uint8_t data)
{
	DDRB |= 0x03;
	DDRD |= 0xFC;
	
	PORTB &= 0xFC;					// Clear PB7:2 in preparation for data.
	PORTD &= 0x03;					// Clear PD1:0.
	
	PORTB |= (data & 0x03);			// Mask off PB1:0 so we don't change them
									//  and then write the other 6 bits.
	PORTD |= (data & 0xFC);			// Mask off PD7:2 so we don't change them
									//	and then write the other two bits.
									//  The data is now in place.
}

uint8_t lcdReadData(void)
{
	DDRB &= ~(0x03);
	DDRD &= ~(0xFC);
	
	uint8_t data = (PINB & 0x03);
	data |= (PIND & 0xFC);
	
	return data;
}