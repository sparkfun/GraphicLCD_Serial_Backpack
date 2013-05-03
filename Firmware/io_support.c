/***************************************************************************
io_support.c

IO support function file for the serial graphical LCD backpack project. This
 file provides an init function for the IO pins, as well as a set/get data
 function which abstracts away the dance you have to do to integrate data
 from two different ports into one byte.

02 May 2013 - Mike Hord, SparkFun Electronics

This code is released under the Creative Commons Attribution Share-Alike 3.0
 license. You are free to reuse, remix, or redistribute it as you see fit,
 so long as you provide attribution to SparkFun Electronics.

***************************************************************************/

#include <avr/io.h>
#include "glcdbp.h"
#include "io_support.h"

// This is setup in glcdbp.c and is used to track the "type" of the display
//  throughout the code.
extern enum DISPLAY_TYPE display;

void ioInit(void)
{
  // Set up the data direction registers for the data bus pins.
  //  The data bus is on PB0:1 and PD2:7, so make those pins outputs.
  DDRB = 0b00001111;
  DDRD = 0b11111100;

  PORTB &= ~(1<<nBL_EN);  // Turn backlight on
  
  // Now we need to configure the I/O to support the two types of display.
  if (display == SMALL)
  {
    DDRC =  ((1<<EN) | (1<<RS) | (1<<R_W) | (1<<RESET) | (1<<CS1) | (1<<CS2));
    PORTC = ((1<<EN) | (1<<RS) | (1<<R_W) | (1<<RESET) | (1<<CS1) | (1<<CS2));
  }
  else if (display == LARGE)
  {
    DDRC =  ((1<<WR) | (1<<RD) | (1<<CE) | (1<<CD) | (1<<HALT) | (1<<RST));
    PORTC = ((1<<WR) | (1<<RD) | (1<<CE) | (1<<CD) | (1<<HALT) | (1<<RST));
  }
}

// Abstracts away the fact that the data pins are not on one port. We *could*
//  inline this code whereever necessary, but who wants to read all this?
void setData(uint8_t data)
{
  // Set the port direction registers to make data pins outputs.
  DDRB |= 0x03;
  DDRD |= 0xFC;
  
  PORTB &= 0xFC;    // Clear PB7:2 in preparation for data.
  PORTD &= 0x03;    // Clear PD1:0.
  
  PORTB |= (data & 0x03); // Mask off PB1:0 so we don't change them
                          //  and then write the other 6 bits.
  PORTD |= (data & 0xFC); // Mask off PD7:2 so we don't change them
                          //  and then write the other two bits.
                          //  The data is now in place.
}

// Same deal as setData.
uint8_t readData(void)
{
  // Set the port direction to input.
  DDRB &= ~(0x03);
  DDRD &= ~(0xFC);
  
  // Pull the data in.
  uint8_t data = (PINB & 0x03);
  data |= (PIND & 0xFC);
  
  return data;
}

// I've noticed that the ks0108b displays don't always play well with their
//  data pins, and to avoid contention and the assorted ugly crap, I've
//  added this function to put the data pins in a hi-z state. I suppose I
//  could just call readData(), but this is easier to read.
void hiZDataPins(void)
{
  // Set the port direction to input.
  DDRB &= ~(0x03);
  DDRD &= ~(0xFC);
}
