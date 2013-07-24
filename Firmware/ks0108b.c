/***************************************************************************
ks0108b.c

Driver support file for the serial graphical LCD backpack project. This file
 adds support for ks0108b-flavored graphical LCDs, providing functions to
 initialize and interface with the controller.

02 May 2013 - Mike Hord, SparkFun Electronics

This code is released under the Creative Commons Attribution Share-Alike 3.0
 license. You are free to reuse, remix, or redistribute it as you see fit,
 so long as you provide attribution to SparkFun Electronics.

***************************************************************************/

#include <avr/io.h>
#include <util/delay.h>    // F_CPU is defined in the makefile
#include "glcdbp.h"
#include "io_support.h"
#include "lcd.h"
#include "ks0108b.h"
#include "serial.h"

#define E_DELAY 5 // This delay is the minimum time EN will be
              //  low or high when enable is strobed. By the
              //  datasheet, this shouldn't need to be more
              //  than 1000ns; experimentation has put the
              //  lie to that. Value in microseconds.
#define R_DELAY 10 // This delay is the delay between EN and
              //  data reads, and between EN changes around
              //  data reads. Again, it shouldn't *need* to
              //  be this long, but the datasheet speaks
              //  great falsehoods. Value in microseconds.
              
uint8_t column = 0; // We want to be able to track the current
              //  x position sometimes; it allows us to pick up where other
              //  functions leave off.
              
extern volatile uint8_t reverse; // Dark-on-light or light-on-dark?
                                 //  Declared in glcdbp.c

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

// Enable the display. Should only need to do this at startup time.
void ks0108bDisplayOn(void)
{
  // Data lines should be 0x3F for display enable.
  PORTC &= ~( (1<<R_W)|      // Clear R_W (Write mode)
              (1<<RS));      // Clear RS (Instruction mode)
  setData(0x3F);
  strobeEN();
  hiZDataPins();     // Avoid bus contention with the ks0108b driver.
  setPinsDefault();
}

// It's possible, if kinda weird, to tell the ks0108b that the start line is
//  not the top line of the display. We're not going to truck with that crap-
//  we'll just start at the top and call it good.
void ks0108bSetStartLine(void)
{
  PORTC &= ~( (1<<CS1)| // Let's hit both chips.
              (1<<CS2)|
              (1<<R_W)|  // Clear R_W (Write mode)
              (1<<RS));  // Clear RS (Register select for
                         //  enable register)
  // Data lines should be 0xC0 for set start line to 0.
  setData(0xC0);
  strobeEN();
  hiZDataPins();     // Avoid bus contention with the ks0108b driver.
  setPinsDefault();
}

// As mentioned elsewhere, this display is divided into 8 meta-rows (or pages,
//  to use the datasheet nomenclature), and within each of those pages are 32
//  columns. This function points us to one of those columns. We'll figure out
//  which half of the screen we're on in the next stage or our operation- that
//  will determine which chip select line we need to assert, since we don't
//  need to assert a CS line here (it wouldn't help anyway).
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
  hiZDataPins();     // Avoid bus contention with the ks0108b driver.
  setPinsDefault();
}

// Select the page (meta-row) that we're currently looking at. This puts BOTH
//  halves of the screen on the same page, so writes that proceed across from
//  L to R can continue across the page boundary without interruption.
void ks0108bSetPage(uint8_t address)
{  
  // Now, on to the actual port manipulations needed to make this happen.
  PORTC &= ~( (1<<R_W)|      // Clear R_W (Write mode)
              (1<<RS));      // Clear RS (Register select for
                             //  address registers)
  // For X writes, bits 7:3 of the data bus should be set to 10111. We should
  //  just make sure that's done before we do anything else...
  address = (address | 0xB8) & 0xBF;
  setData(address);
  strobeEN();
  hiZDataPins();     // Avoid bus contention with the ks0108b driver.
  setPinsDefault();
}
// ks0108bWriteData- write a data byte to the controller. This is only for
//  writing data that is expected to appear on screen, but then, that's
//  really *all* this LCD lets you write data for!
void ks0108bWriteData(uint8_t data)
{  
  // By tracking what column we're writing to, we can avoid having to
  //  do any weird "which side am I on" logic, keeping the interface more
  //  intuitive.
  if (column < 64)   PORTC &= ~( (1<<CS1) |
                                 (1<<R_W));
  else               PORTC &= ~( (1<<CS2) |
                                 (1<<R_W));
  
  // setData() is a function which abstracts the fact that the data lines
  //  to the LCD are not on the same port.
  setData(data);
  
  strobeEN();        // Twiddle EN to latch the data.
  hiZDataPins();     // Avoid bus contention with the ks0108b driver.
  setPinsDefault();
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

// Read a column of pixel data. The operation is basically thus:
//   1. Pull one CS line and EN low
//   2. Pull EN high.
//   3. Pull EN low.
//   4. Pull EN high.
//   5. Data is available to be read.
//   6. Reset signal lines to rest state.
uint8_t ks0108bReadData(uint8_t x)
{  
  uint8_t data;
  
  if (x<64)  // Are we on the left half of the display?
  {  
    PORTC &= ~(1<<CS1);
  }
  else       // Or the right half of the display?
  {  
    PORTC &= ~(1<<CS2);
  }
  // The number of twiddles of EN is...bizarre. This was established via
  //  experimentation, rather than through any actual data sheet content.
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
  setPinsDefault();
  return data;
}

// Clear is janky- set x and y to zero and write across the screen.
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

// This is the display-specific pixel draw command. Pretty simple- located the
//  pixel's row and column, read the existing data, twiddle the single pixel
//  according to what we want it to end up being, then re-write the whole
//  byte.
void ks0108bDrawPixel(uint8_t x, uint8_t y, PIX_VAL pixel)
{
  ks0108bSetColumn(x);  // x is simple; it's just the x coordinate.
  ks0108bSetPage(y/8);  // y is less simple; we need to find the page that the
                        //  pixel in question resides on.
  uint8_t currentPixelData = ks0108bReadData(x);  // fetch the existing state
  uint8_t pixelToWrite = (y%8);  // determine which pixel to write
  // This section handles the specifics- do we want to turn the pixel on or
  //  off? The dark-on-white mode status factors into that, as does the user's
  //  command.
  if (reverse == 0)
  {
    if (pixel == ON) currentPixelData |= (1<<pixelToWrite);
    else       currentPixelData &= ~(1<<pixelToWrite);
  }
  else
  {
    if (pixel == OFF) currentPixelData |= (1<<pixelToWrite);
    else       currentPixelData &= ~(1<<pixelToWrite);
  }
  // Now we need to go back to the original column- the read incremented the
  //  address counter- and write the changed value back to the display.
  ks0108bSetColumn(x);
  ks0108bWriteData(currentPixelData);
}

// I found myself typing these lines over and over, so I made them a little
//  function of their very own.
void strobeEN(void)
{
  _delay_us(E_DELAY);
  PORTC |= (1<<EN);        // Set EN (Indicate data ready)
  _delay_us(E_DELAY);
  PORTC &= ~(1 << EN);      // Clear EN (Activate write)
  _delay_us(E_DELAY);
}

// Everytime we finish up a transfer, we want to reset the pins to a default
//  state. This state is all pins high EXCEPT EN. We want to leave EN low
//  until we need to twiddle it.
void setPinsDefault(void)
{
  PORTC |= (  (1<<RS)| 
              (1<<CS1)|
              (1<<CS2)|
              (1<<R_W));
  PORTC &= ~(1<<EN);
}