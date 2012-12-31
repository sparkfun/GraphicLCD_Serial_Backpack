#include <avr/io.h>
#include <util/delay.h>
#include "ks0108b.h"
#include "glcdbp.h"
#include "lcd.h"

#define E_DELAY 5
uint8_t column = 0;

void ks0108bReset(void)
{
	PORTC &= ~(1<<RESET);
	_delay_ms(50);
	PORTC |= (1<<RESET);
	_delay_ms(50);
}

void ks0108bBusyWait(void)
{
}

// Write a data byte to the controller. Data lines 7:2 are connected to pins
//	B7:2 and 1:0 are on D1:0.
void ks0108bWriteData(uint8_t data)
{
	lcdSetData(data);
	if (column++ > 63) 	PORTC &= ~( (1<<CS2) );
	else 			 	PORTC &= ~( (1<<CS1) );
	PORTC &= ~( (1<<R_W));
	strobeEN();
	PORTC |=  ( (1<<R_W)|
				(1<<CS1)|
				(1<<CS2));
	if (column > 127) 
	{
		column = 0;
		ks0108bSetColumn(0);
	}
}

uint8_t ks0108bReadData(void)
{	
	uint8_t data=lcdReadData();
	PORTC &= ~( (1<<CS1));
	_delay_us(E_DELAY);
	PORTC |= (1<<EN);
	_delay_us(E_DELAY);
	data = lcdReadData();
	PORTC &= ~(1<<EN);
	PORTC |= (1<<CS1);	
	return data;
}

void ks0108bSetColumn(uint8_t address)
{	
	PORTC &= ~( (1<<R_W)|
				(1<<RS));
	// For Y writes, bits 7:6 of the data bus should be set to 01. We should
	//  just make sure that's done before we do anything else...
	address = (address | 0x40) & 0x7F;
	lcdSetData(address);
	strobeEN();
	PORTC |=  ( (1<<R_W)|
				(1<<RS));
}

void ks0108bSetPage(uint8_t address)
{	
	// Now, on to the actual port manipulations needed to make this happen.
	PORTC &= ~( (1<<R_W)|			// Clear R_W (Write mode)
				(1<<RS));			// Clear RS (Register select for
									//  address registers)
	// For X writes, bits 7:3 of the data bus should be set to 10111. We should
	//  just make sure that's done before we do anything else...
	address = (address | 0xB8) & 0xBF;
	lcdSetData(address);
	strobeEN();
	PORTC |= (	(1<<R_W)|
				(1<<RS));
}

uint8_t ks0108bReadStatus(void)
{	
	uint8_t status = lcdReadData();
	PORTC &= ~((1<<CS1)|
			   (1<<RS));
	strobeEN();
	strobeEN();
	status = lcdReadData();
	PORTC |= ((1<<CS1)|
			  (1<<RS));
	return status;
}

void ks0108bDisplayOn(void)
{
	// Data lines should be 0x3F for display enable.
	PORTC &= ~(	(1<<R_W)|			// Clear R_W (Write mode)
				(1<<RS));			// Clear RS (Instruction mode)
	lcdSetData(0x3F);
	strobeEN();
	PORTC |= (	(1<<R_W)|			// Set R_W
				(1<<RS));			// Set RS
}

void ks0108bDisplayOff(void)
{
}

void ks0108bSetStartLine(void)
{
	PORTC &= ~(	(1<<CS1)|
				(1<<CS2)|
				(1<<R_W)|			// Clear R_W (Write mode)
				(1<<RS));			// Clear RS (Register select for
									//  enable register)
	// Data lines should be 0xC0 for set start line to 0.
	lcdSetData(0xC0);
	strobeEN();
	PORTC |= (	(1<<R_W)|			// Set R_W
				(1<<RS)|			// Set RS
				(1<<CS1)|
				(1<<CS2));
}

void ks0108bDrawPixel(uint8_t x, uint8_t y, PIX_VAL pixel)
{
	ks0108bSetColumn(x);
	ks0108bSetPage(y/8);
	uint8_t currentPixelData = ks0108bReadData();
	uint8_t pixelToWrite = (y%8);
	if (pixel == ON) currentPixelData |= (1<<pixelToWrite);
	else			   currentPixelData &= ~(1<<pixelToWrite);
	ks0108bSetColumn(x);
	ks0108bWriteData(currentPixelData);
}

void ks0108bClear(void)
{
	for (uint8_t y = 0; y<8; y++)
	{
		ks0108bSetPage(y);
		ks0108bSetColumn(0);
		for (uint8_t x = 0; x<128; x++)
		{
			ks0108bWriteData(0);
		}
	}
	ks0108bSetPage(0);
	ks0108bSetColumn(0);
}

void strobeEN(void)
{
	_delay_us(E_DELAY);
	PORTC |= (1<<EN);				// Set EN (Indicate data ready)
	_delay_us(E_DELAY);
	PORTC &= ~(1 << EN);			// Clear EN (Activate write)
	_delay_us(E_DELAY);
}