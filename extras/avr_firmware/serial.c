/***************************************************************************
serial.c

Serial functionality file for the serial graphical LCD backpack project.
 All the various trappings of serial port support- writing data out,
 initializing the hardware, accessing the buffer- are here.

02 May 2013 - Mike Hord, SparkFun Electronics

This code is released under the Creative Commons Attribution Share-Alike 3.0
 license. You are free to reuse, remix, or redistribute it as you see fit,
 so long as you provide attribution to SparkFun Electronics.

***************************************************************************/


#include <avr/io.h>
#include "serial.h"
#include "glcdbp.h"

// These variables are defined in glcdbp.c, and are used for the input buffer
//   from the serial port. We need to be able to access them here because we'll
//   want to abstract popping from the buffer to a function.
extern volatile uint8_t   rxRingBuffer[BUF_DEPTH];
extern volatile uint16_t  rxRingHead;
extern volatile uint16_t  rxRingTail;
extern volatile uint8_t   bufferSize;

// Initialize the serial port hardware.
void serialInit(uint16_t baudRate)
{
  // Set baud rate 
  UBRR0 = baudRate;

  // Enable receiver and transmitter 
  UCSR0A = (1<<U2X0);
  UCSR0B = (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0);  //Enable Interrupts on receive

  UCSR0C = (1<<UCSZ00)|(1<<UCSZ01);
}

// A simple function that waits for the clear to send from the USART, then
//  dumps out a data byte. All other serial puts are based on this.
void putChar(uint8_t TXData)
{
  while ( !( UCSR0A & (1<<UDRE0)) );  // Wait for empty transmit buffer
  UDR0 = TXData;    // Put data into buffer; uC automatically
                    //  sends data once it's written.
}

// I probably didn't need to write this, but I did. Converts an 8-bit number
//  to two-digit hex and prints it.
void putHex(uint8_t TXData)
{
  uint8_t hexChars[2]; // We'll use an array to buffer the characters.
  // First, let's isolate the two halves of the byte in question.
  hexChars[0] = (0x0F) & TXData;
  hexChars[1] = ((0xF0) & TXData)>>4;
  // Okay, now we can convert them to an ASCII value.
  if (hexChars[1] < 10) hexChars[1] += '0';
  else hexChars[1] += ('A'-10);
  putChar(hexChars[1]);
  if (hexChars[0] < 10) hexChars[0] += '0';
  else hexChars[0] += ('A'-10);
  putChar(hexChars[0]);
}

// Convert an 8-bit value into a three-digit decimal number and print it.
void putDec(uint8_t TXData)
{
  uint8_t ones = TXData % 10;
  TXData -= ones;
  uint8_t tens = (TXData % 100)/10;
  uint8_t huns = TXData / 100;
  putChar((huns) + '0');
  putChar((tens) + '0');
  putChar((ones) + '0');
}

// Convert an 8-bit value into an 8-bit binary number and print it.
void putBin(uint8_t TXData)
{
  for (char i = 0; i < 8; i++)
  {
    if ((TXData & 0x01) == 0x01) putChar('1');
    else putChar('0');
    TXData = TXData>>1;
  }
}

// Print a string, followed by carriage return and new line.
void putLine(char *TXData)
{
  while (*TXData != '\0')
  {
    putChar(*(TXData++));
  }
  putChar('\n');
  putChar('\r');
}

// Grab the top byte off the serial FIFO and return it, adjusting the pointers
//  and size of the FIFO accordingly.
char serialBufferPop(void)
{
  bufferSize--;
  char retVal = rxRingBuffer[rxRingTail++];
  if (rxRingTail == BUF_DEPTH) rxRingTail = 0;
  return retVal;
}

// Clear the FIFO buffer. Note that this doesn't actually delete the info in
//  the buffer, it just resets the size and the pointers.
void clearBuffer(void)
{
  bufferSize = 0;
  rxRingTail = 0;
  rxRingHead = 0;
}
