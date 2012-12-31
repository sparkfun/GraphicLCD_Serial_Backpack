#include <avr/io.h>
#include <util/delay.h>
#include "ks0108b.h"
#include "lcd.h"
#include "glcdbp.h"

#define E_DELAY 20

void ks0108bReset(void)
{
	PORTC &= ~(1<<RESET);
	_delay_ms(50);
	PORTC |= (1<<RESET);
}

void ks0108bBusyWait(void)
{
	DDRB &= ~(0x03);
	DDRD &= ~(0xFC);
	PORTC &= ~(	(1<<CS1)|
				(1<<RS) |
				(1<<EN));
	_delay_us(E_DELAY);
	PORTC |= (1<<EN);
	_delay_us(E_DELAY);
	while ((PIND & 0x90) != 0)
	{
	}
	PORTC &= ~((1<<EN));
	_delay_us(E_DELAY);
	PORTC |= (	(1<<CS1)|
				(1<<RS)|
				(1<<EN));
	_delay_us(E_DELAY);
}

// Write a data byte to the controller. Data lines 7:2 are connected to pins
//	B7:2 and 1:0 are on D1:0.
void ks0108bWriteData(uint8_t data)
{
	LATrigger();
	//ks0108bBusyWait();
	lcdSetData(data);
	_delay_us(E_DELAY);
	PORTC &= ~( (1<<R_W)|
				(1<<CS1)|
				(1<<EN));
	_delay_us(E_DELAY);
	PORTC |= (1 << EN);
	_delay_us(E_DELAY);
	PORTC &= ~((1<<EN));
	_delay_us(E_DELAY);
	PORTC |=  ( (1<<R_W)|
				(1<<CS1)|
				(1<<EN));
	LATrigger();
}

uint8_t ks0108bReadData(void)
{	
	uint8_t data=0;
	return data;
}

void ks0108bSetColumn(uint8_t address)
{	
	//ks0108bBusyWait();
	// For Y writes, bits 7:6 of the data bus should be set to 01. We should
	//  just make sure that's done before we do anything else...
	address &= 0b00111111;
	address |= 0x01000000;
	lcdSetData(address);
	_delay_us(E_DELAY);
	// Now, on to the actual port manipulations needed to make this happen.
	PORTC &= ~( (1<<EN)|			// Clear EN (Activate write)
				(1<<R_W)|			// Clear R_W (Write mode)
				(1<<RS));			// Clear RS (Register select for
									//  address registers)
	_delay_us(E_DELAY);
	PORTC |= (1<<EN);
	_delay_us(E_DELAY);
	PORTC &= ~(1<<EN);
	_delay_us(E_DELAY);
	PORTC |= (	(1<<RS)|			// Set RS
				(1<<R_W)|			// Set R_W
				(1<<EN));			// Set EN (Indicate data ready)
}

void ks0108bSetPage(uint8_t address)
{	
	//ks0108bBusyWait();
	// For X writes, bits 7:3 of the data bus should be set to 10111. We should
	//  just make sure that's done before we do anything else...
	address &= 0b00000111;
	address |= 0b10111000;
	lcdSetData(address);
	_delay_us(E_DELAY);
	// Now, on to the actual port manipulations needed to make this happen.
	PORTC &= ~( (1<<EN)|			// Clear EN (Activate write)
				(1<<R_W)|			// Clear R_W (Write mode)
				(1<<RS));			// Clear RS (Register select for
									//  address registers)
	_delay_us(E_DELAY);
	PORTC |= (1<<EN);				// Set EN (Indicate data ready)
	_delay_us(E_DELAY);
	PORTC &= ~(1 << EN);			// Clear EN (Activate write)
	_delay_us(E_DELAY);
	PORTC |= (	(1<<EN)|				
				(1<<R_W)|
				(1<<RS));
}

uint8_t ks0108bReadStatus(void)
{	
	ks0108bBusyWait();
	uint8_t status;
	PORTC &= ~((1<<CS1)|
			   (1<<RS)|
			   (1<<EN));
	_delay_us(E_DELAY);
	PORTC |= (1<<EN);
	_delay_us(E_DELAY);
	PORTC &= ~(1<<EN);
	_delay_us(E_DELAY);
	status = lcdReadData();
	PORTC |= ((1<<CS1)|
			  (1<<RS)|
			  (1<<EN));
	return status;
}

void ks0108bDisplayOn(void)
{
	//ks0108bBusyWait();
	// Data lines should be 0x3F for display enable.
	lcdSetData(0x3F);
	_delay_us(E_DELAY);
	PORTC &= ~(	(1<<EN)|			// Clear EN (Activate write)
				(1<<R_W)|			// Clear R_W (Write mode)
				(1<<RS));			// Clear RS (Register select for
									//  enable register)
	// Data lines should be 0x3F for display enable.
	_delay_us(E_DELAY);
	PORTC |= (1<<EN);
	_delay_us(E_DELAY);
	PORTC &= ~(1 << EN);			// Clear EN (Activate write)
	_delay_us(E_DELAY);
	PORTC |= (	(1<<EN)|			// Set EN (Indicate data ready)
				(1<<R_W)|			// Set R_W
				(1<<RS));			// Set RS
}

void ks0108bDisplayOff(void)
{

}

void ks0108bSetStartLine(void)
{
	//ks0108bBusyWait();
	// Data lines should be 0xC0 for set start line to 0.
	lcdSetData(0xC0);
	_delay_us(E_DELAY);
	PORTC &= ~(	(1<<EN)|			// Clear EN (Activate write)
				(1<<R_W)|			// Clear R_W (Write mode)
				(1<<RS));			// Clear RS (Register select for
									//  enable register)
	// Data lines should be 0xC0 for set start line to 0.
	_delay_us(E_DELAY);
	PORTC |= (1<<EN);				// Set EN (Indicate data ready)
	_delay_us(E_DELAY);
	PORTC &= ~(1 << EN);			// Clear EN (Activate write)
	_delay_us(E_DELAY);
	PORTC |= (	(1<<R_W)|			// Set R_W
				(1<<RS)|			// Set RS
				(1<<EN));			// Set EN (Indicate data ready)
}
