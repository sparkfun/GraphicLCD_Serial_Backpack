#include <avr/io.h>
#include "t6963.h"

// Write a data byte to the controller. Data lines 7:2 are connected to pins
//	B7:2 and 1:0 are on D1:0.
void t6963WriteData(uint8_t data)
{
	PORTB |= data & 0xFC;			// Mask off PB1:0 so we don't change them
									//  and then write the other 6 bits.
	PORTD |= data & 0x03;			// Mask off PD7:2 so we don't change them
									//	and then write the other two bits.
									//  The data is now in place.
	PORTC &= !((1<<CD) | (1<<WR));	// Make CD and WR low, without affecting
									//	the rest of the pins on Port C.
									//  CD is command/!data (low indicates
									//  data), WR is active low write mode.
	PORTC |= (1<<RD);				// Make sure RD is high; this disables read
									//  mode for the chip.
	PORTC &= !(1<<CE);				// Strobe the chip enable low, then high,
	PORTC |= (1<<CE);				//  to execute the write.
}

uint8_t t6963ReadData(void)
{	
	uint8_t data;
	PORTC &= !((1<<CD) | (1<<RD));	// Make CD and RD low, without affecting
									//	the rest of the pins on Port C.
									//  CD is command/!data (low indicates
									//  data), RD is active low read mode.
	PORTC |= (1<<WR);				// Make sure WR is high; this disables
									//  write mode for the chip.
	PORTC &= !(1<<CE);				// Strobe the chip enable low.
	data = PINB & 0xFC;				// Mask off PB1:0 so we don't change them
									//  and then write the other 6 bits.
	data |= PIND & 0x03;			// Mask off PD7:2 so we don't change them
									//	and then write the other two bits.
									//  The data is now in place.
	PORTC |= (1<<CE);				// Pull CE high again to finish.
}

void t6963WriteCmd(uint8_t command)
{	
	PORTB |= command & 0xFC;		// Mask off PB1:0 so we don't change them
									//  and then write the other 6 bits.
	PORTD |= command & 0x03;		// Mask off PD7:2 so we don't change them
									//	and then write the other two bits.
									//  The data is now in place.
	PORTC &= !(1<<WR);				// Make WR low, without affecting
									//	the rest of the pins on Port C.
									//  WR is active low write mode.
	PORTC |= ( (1<<RD) | (1<<CD) );	// Make sure RD is high; this disables read
									//  mode for the chip. Also make sure CD is
									//  high, to indicate command mode.
	PORTC &= !(1<<CE);				// Strobe the chip enable low, then high,
	PORTC |= (1<<CE);				//  to execute the write.
}

uint8_t t6963ReadStatus(void)
{	
	uint8_t status;
	PORTC &= !(1<<RD);				// Make RD low, without affecting
									//	the rest of the pins on Port C.
									//  RD is active low read mode.
	PORTC |= ((1<<WR)|(1<<CD));		// Make sure WR is high; this disables
									//  write mode for the chip. Also, make CD
									//  high to indicate command mode.
	PORTC &= !(1<<CE);				// Strobe the chip enable low.
	status = PINB & 0xFC;				// Mask off PB1:0 so we don't change them
									//  and then write the other 6 bits.
	status |= PIND & 0x03;			// Mask off PD7:2 so we don't change them
									//	and then write the other two bits.
									//  The data is now in place.
	PORTC |= (1<<CE);				// Pull CE high again to finish.
}
