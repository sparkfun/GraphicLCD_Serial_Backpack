#include <avr/io.h>
#include "io_support.h"

void ioInit(void)
{
	// Set up the data direction registers for the data bus pins.
	//  The data bus is on PB0:1 and PD2:7, so make those pins outputs.
	DDRB = 0b00001111;
	DDRD = 0b11111100;

	PORTB &= ~(1<<nBL_EN);		// Turn backlight on
	
	// Now we need to configure the I/O to support the two types of display.
	//if (display == SMALL)
	{
		DDRC =  ((1<<EN) | (1<<RS) | (1<<R_W) | (1<<RESET) | (1<<CS1) | (1<<CS2));
		PORTC = ((1<<EN) | (1<<RS) | (1<<R_W) | (1<<RESET) | (1<<CS1) | (1<<CS2));
	}
	/*else if (display == LARGE)
	{
		DDRC =  ((1<<WR) | (1<<RD) | (1<<CE) | (1<<CD) | (1<<HALT) | (1<<RST));
		PORTC = ((1<<WR) | (1<<RD) | (1<<CE) | (1<<CD) | (1<<HALT) | (1<<RST));
	}*/
}

void LATrigger(void)
{
	PORTB |= 0x08;
	PORTB &= ~(0x08);
}

void setData(uint8_t data)
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

uint8_t readData(void)
{
	DDRB &= ~(0x03);
	DDRD &= ~(0xFC);
	
	uint8_t data = (PINB & 0x03);
	data |= (PIND & 0xFC);
	
	return data;
}
