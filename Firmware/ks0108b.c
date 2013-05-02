#include <avr/io.h>
#include <util/delay.h>		// F_CPU is defined in the makefile
#include "glcdbp.h"
#include "io_support.h"
#include "lcd.h"
#include "ks0108b.h"
#include "serial.h"

#define E_DELAY 5			// This delay is the minimum time EN will be
							//  low or high when enable is strobed. By the
							//  datasheet, this shouldn't need to be more
							//  than 1000ns; experimentation has put the
							//  lie to that. Value in microseconds.
#define R_DELAY 8			// This delay is the delay between EN and
							//  data reads, and between EN changes around
							//  data reads. Again, it shouldn't *need* to
							//  be this long, but the datasheet speaks
							//  great falsehoods. Value in microseconds.
							
uint8_t column = 0;			// We want to be able to track the current
							//  x position sometimes; it allows 
              
extern volatile uint8_t reverse;

// ks0108bReset()- pretty self explanatory, but I'm not really sure what
//  the point of twiddling the reset line is, as it doesn't seem to really
//  *reset* anything on the display. Makes us feel good, though.
void ks0108bReset(void)
{
	PORTC &= ~(1<<RESET);
	_delay_ms(50);
	PORTC |= (1<<RESET);
	_delay_ms(50);
}

// ks0108bWriteData- write a data byte to the controller. This is only for
//  writing data that is expected to appear on screen, but then, that's
//  really *all* this LCD lets you write data for!
void ks0108bWriteData(uint8_t data)
{
	// setData() is a function which abstracts the fact that the data lines
	//  to the LCD are not on the same port.
	setData(data);
	
	// By tracking what column we're writing to, we can avoid having to
	//  do any weird "which side am I on" logic, keeping the interface more
	//  intuitive.
	if (column > 63) 	PORTC &= ~( (1<<CS2) );
	else 			 	PORTC &= ~( (1<<CS1) );
	// R_W goes low, for a write.
	PORTC &= ~( (1<<R_W));
	strobeEN();				// Twiddle EN to latch the data.
	PORTC |=  ( (1<<R_W)|	// Reset all our pins to the default
				(1<<CS1)|	//  rest state of all high.
				(1<<CS2)|
				(1<<EN));
	// The act of writing a data byte to the display causes the display's
	//  internal pointer to increment. We need to update our pointer to
	//  account for that, but if the update pushes our pointer past the
	//  edge, we want to wrap back around. The display doesn't handle
	//  wrapping automatically, so we need to wrap it manually with the
	//  set column function.
	if (++column > 127) 
	{
		column = 0;
		ks0108bSetColumn(0);
	}
}

// ks0108bReadBlock()- reads an 8x8 block of arbitrary pixels from the display.
//  The block may be split across more than one page, so we'll need to buffer
//  from up to two pages, then do some shifting.
void ks0108bReadBlock(uint8_t x, uint8_t y, uint8_t *buffer)
{
  uint8_t firstRowPixels, secondRowPixels;
  // If y is NOT divisible by 8, then y%8 pixels are on the first line,
  //  and 8-y%8 pixels are on the second line.
  firstRowPixels = y%8;
  secondRowPixels = 8 - (y%8);
  // Okay, now we know how many pixels are in each row. Now let's pull the
  //  data from those two rows.
	ks0108bSetPage(y/8);
  for (uint8_t i = 0; i<8; i++)
  {
    // Fetch the data and left-shift it so the topmost pixel of the group
    //  we're interested in is the MSB.
    buffer[i] = ks0108bReadData(x+i)<<(8-firstRowPixels);
  }
  ks0108bSetPage((y/8) + 1);
  for (uint8_t i = 0; i<8; i++)
  {
    buffer[i] |= ks0108bReadData(x+i)>>(8-secondRowPixels);
  }
}

uint8_t ks0108bReadData(uint8_t x)
{	
	uint8_t data;
	
	if (x<64) 
	{	
		PORTC &= ~( (1<<CS1)|
					(1<<EN) );
	}
	else
	{	
		PORTC &= ~( (1<<CS2)|
					(1<<EN) );
	}
	_delay_us(R_DELAY);
	PORTC |= (1<<EN);	
	_delay_us(R_DELAY);
	PORTC &= ~(1<<EN);
	_delay_us(R_DELAY);
	PORTC |= (1<<EN);	
	_delay_us(R_DELAY);
	data = readData();	
	PORTC &= ~(1<<EN);
	_delay_us(R_DELAY);
	PORTC |= ( (1<<CS1)|
			   (1<<CS2)|
			   (1<<EN) );	
	_delay_us(R_DELAY);
	return data;
}

void ks0108bSetColumn(uint8_t address)
{	
	PORTC &= ~( (1<<R_W)|
				(1<<RS));
	// For Y writes, bits 7:6 of the data bus should be set to 01. We should
	//  just make sure that's done before we do anything else...
	column = address;
	address = (address | 0x40) & 0x7F;
	setData(address);
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
	setData(address);
	strobeEN();
	PORTC |= (	(1<<R_W)|
				(1<<RS));
}

uint8_t ks0108bReadStatus(void)
{	
	uint8_t status = readData();
	PORTC &= ~((1<<CS1)|
			   (1<<RS));
	strobeEN();
	strobeEN();
	status = readData();
	PORTC |= ((1<<CS1)|
			  (1<<RS));
	return status;
}

void ks0108bDisplayOn(void)
{
	// Data lines should be 0x3F for display enable.
	PORTC &= ~(	(1<<R_W)|			// Clear R_W (Write mode)
              (1<<RS));			// Clear RS (Instruction mode)
	setData(0x3F);
	strobeEN();
	PORTC |= ( (1<<R_W)|			// Set R_W
              (1<<RS));			// Set RS
}

void ks0108bSetStartLine(void)
{
	PORTC &= ~(	(1<<CS1)|
              (1<<CS2)|
              (1<<R_W)|			// Clear R_W (Write mode)
              (1<<RS));			// Clear RS (Register select for
									//  enable register)
	// Data lines should be 0xC0 for set start line to 0.
	setData(0xC0);
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
	uint8_t currentPixelData = ks0108bReadData(x);
	uint8_t pixelToWrite = (y%8);
  if (reverse == 0)
  {
    if (pixel == ON) currentPixelData |= (1<<pixelToWrite);
    else			 currentPixelData &= ~(1<<pixelToWrite);
  }
  else
  {
    if (pixel == OFF) currentPixelData |= (1<<pixelToWrite);
    else			 currentPixelData &= ~(1<<pixelToWrite);
  }
	ks0108bSetColumn(x);
	ks0108bWriteData(currentPixelData);
}

void ks0108bClear(void)
{
  uint8_t clearVal = 0;
  if (reverse == 1) clearVal = 0xFF;
	for (uint8_t y = 0; y<8; y++)
	{
		ks0108bSetPage(y);
		ks0108bSetColumn(0);
		for (uint8_t x = 0; x<128; x++)
		{
			ks0108bWriteData(clearVal);
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
