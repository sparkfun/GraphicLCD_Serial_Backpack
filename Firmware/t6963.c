/***************************************************************************
t6963.c

t6963 controller driver file. Handles the hardware-level interfacing for the
 t6963 LCD controller.

02 May 2013 - Mike Hord, SparkFun Electronics

This code is released under the Creative Commons Attribution Share-Alike 3.0
 license. You are free to reuse, remix, or redistribute it as you see fit,
 so long as you provide attribution to SparkFun Electronics.

***************************************************************************/

#include <avr/io.h>
#include <util/delay.h>		// F_CPU is defined in the makefile
#include "glcdbp.h"
#include "serial.h"
#include "io_support.h"
#include "t6963.h"

extern volatile uint8_t reverse; // This is defined in glcdbp.c

// Basic functionality: clearing the display. All we're *really* doing is 
//  writing a one or zero to all the memory locations for the display.
void t6963Clear(void)
{
  // Begin at the beginning.
  t6963SetPointer(0,0);
  // Then, clear out the whole enchilada. We *could* use the auto increment 
  //  mode here to shorten this up a bit, but meh. That would save us the 
  //  command write for each loop, and reduce the overall time by probably a 
  //  few milliseconds.
  for (uint16_t i = 0; i < 2560; i++)
  {
    if (reverse) t6963WriteData(0xff);
    else         t6963WriteData(0x00);
    t6963WriteCmd(0xc0);  // This is the "write-data-then-increment-pointer"
                          //  command. Useful.
  }
}

// Write a data byte to the controller.
void t6963WriteData(uint8_t data)
{
  t6963BusyWait(); // Wait for the controller to be ready.
  setData(data);   // Set up the data onto the lines.
  PORTC &= ~(1<<CD); // This tells the controller that we are sending DATA, not
                     //  a command
  _delay_us(1);    // Our processor speed puts us on the hairy edge of the
                   //  timing, so we pad a things a bit here and there.
  PORTC &= ~((1<<WR) | // WRITE, not read, and enable the chip.
             (1<<CE));
  _delay_us(1);    // Again, let's keep our hold time legit.
  PORTC |= (1<<CE); // De-assert the chip.
  PORTC |= ((1<<CD) | // Finally, put the CD, WR, and RD pins back to a known
            (1<<WR) | //  state. We'll do this after every basic command.
            (1<<RD));
}

// Read a data byte from the controller.
uint8_t t6963ReadData(void)
{  
  t6963BusyWait();  // Wait for controller to be ready.
  uint8_t data;
  PORTC &= ~(1<<CD); // This is a DATA transaction.
  _delay_us(1);      // Hold time.
  PORTC &= ~( (1<<CE) |  // Enable the chip...
              (1<<RD) ); // ...and tell it this is a READ.
  _delay_us(1);      // Hold time.
  data = readData(); // Fetch the data.
  PORTC |= (1<<CE);  // De-select the chip.
  PORTC |= ((1<<CD) | // Go back to our known state for the signal lines.
            (1<<WR) |
            (1<<RD));
  return data;
}

// Write a command to the controller. Note that "reading" a command is
//  nonsensical and no ReadCommand() function is provided.
void t6963WriteCmd(uint8_t command)
{  
  t6963BusyWait();   // Wait for controller to be ready.
  setData(command);  // Set up the data on the lines.
  _delay_us(1);      // Hold time.
  PORTC &= ~(1<<WR); // Tell the controller that we're WRITING.
  _delay_us(1);      // Hold time.
  PORTC &= ~(1<<CE); // Enable the controller to complete the write.
  _delay_us(1);      // Hold time, for the write to happen.
  PORTC |= (1<<CE);  // Deselect the chip.
  PORTC |= ((1<<CD) | // Put the lines back to known normal state.
            (1<<WR) |
            (1<<RD));
}

// Read the current chip status. Note that writing the status is not allowed.
uint8_t t6963ReadStatus(void)
{  
  // Astute readers will note that there is *no* busyWait() here! This is the
  //  one basic operation that does not, indeed, CANNOT, busy wait, since
  //  busy waiting requires one to read the status of the chip.
  uint8_t status;
  PORTC &= ~(1<<RD);  // We're reading.
  _delay_us(1);       // Hold time.
  PORTC &= ~(1<<CE);  // Enable the chip.
  _delay_us(1);       // Hold time.
  status = readData(); // The status should be on the data pins; fetch it.
  PORTC |= (1<<CE);   // Deselect the chip.
  PORTC |= ((1<<CD) | // Housekeeping- put the pins back to known rest state.
            (1<<WR) |
            (1<<RD));
  return status;      // Return our findings.
}

// busyWait() spins its wheels until the controller returns that it's ready to
//  accept a new data or command byte. Note that this is *not* applicable for
//  the auto-write mode, only for normal command and data read writes.
//  Bits 1:0 of the status result will both be zero when interaction with the
//  controller is counterindicated. We want to wait for a nonzero value to
//  appear there.
void t6963BusyWait(void)
{
  uint8_t status;
  do
  {
    status = t6963ReadStatus();
  } while ((status & 0x03) == 0x00);
}

// Set the pointer to the byte which contains an arbirary x, y point. For our
//  160 x 128 pixel display, there are 20*128 memory address, so we need a
//  16-bit value to refer to the whole graphics area array.
void t6963SetPointer(uint8_t x, uint8_t y)
{
  // Calculate which address in our memory space contains the pixel. For each
  //  increase in y, we increase by 20 locations. For each 8 pixels in x, we
  //  increase by one location. Using a 3-right-shift is a cheap way of doing
  //  divide by 8 in a processor without a divide operation. Maybe the
  //  compiler knows that, maybe not.
  uint16_t pointerAddress = (y * 20) + (x>>3);
  // Now that we have our address, we can write our data out.
  // This is the low byte of the address
  t6963WriteData((uint8_t)pointerAddress);
  // This is the high byte of the address
  t6963WriteData((uint8_t)(pointerAddress>>8));
  t6963WriteCmd(0x24);  // This is the command for "set pointer address".
}

void t6963DisplayInit(void)
{
  // The first part of display initialization is to set the start location of
  //  the graphics in memory. We'll set it to 0x0000.
  t6963WriteData(0x00); // Write the low byte of the graphics home address.
  t6963WriteData(0x00); // Write the high byte of the graphics home address.
  t6963WriteCmd(0x42);  // "Write graphics home address" command.
  
  // Next, we need to set the graphics area. This is the length of each line
  //  before the line wraps to the next one. Note that it does not have to
  //  equal the actual number of pixels in the display- just equal to or
  //  greater than.
  t6963WriteData(20);   // # bytes per line (160 pixels/8 bits per byte)
  t6963WriteData(0x00); // Always zero.
  t6963WriteCmd(0x43);  // "Write graphics area" command.
  
  // Now we need to write the mode set command; most likely, this is not
  //  needed, because the defaults should work, but never trust the defaults.
  //  Also, I *think* this only really affects the way text is rendered, and
  //  since we're only doing graphics rendering, it probably doesn't matter.
  //  Register takes the form
  //    1  0  0  0  CG  MD2  MD1  MD0
  //  CG -    0   = internal ROM character generation
  //          1   = RAM character generation
  //  MD2-0 - 000 = OR mode
  //          001 = XOR mode
  //          010 = AND mode
  //          100 = TEXT ATTRIBUTE mode
  t6963WriteCmd(0x80);
  
  // There's a DISPLAY MODE command, too. This seems more likely to be useful
  //  to us. The register takes the form
  //     1  0  0  1  GRPH  TEXT  CUR  BLK
  //  GRPH - 1/0 graphics on/off
  //  TEXT - 1/0 text display on/off
  //  CUR  - 1/0 text cursor displayed/not displayed
  //  BLK  - 1/0 text cursory blink on/off
  t6963WriteCmd(0x98);
}

// In addition to bytewise read/write of data, the t6963 can do a bitwise
//  set/reset of pixels natively. To do this, we use this command:
//    1  1  1  1  S/R  B2  B1  B0
//  S/R   -  1/0 Set/reset bit
//  B2:B0 -  index of bit to be set or reset (value is in bitIndex)
void t6963BitSR(uint8_t bit, uint8_t SR)
{
  uint8_t command = 0xf0;
  command |= SR;
  command |= bit;
  t6963WriteCmd(command);
}

// To draw a single pixel, we need to set the address pointer to the byte
//  which contains the pixel, then issue a "bit set/clear" command on that
//  bit. Note that the order of pixels is backwards- while the address of
//  the columns increases across the display, within a given 8-bit column,
//  bit 0 is the rightmost bit.
void t6963DrawPixel(uint8_t x, uint8_t y, PIX_VAL pixel)
{
  // Step one: select the byte in question.
  t6963SetPointer(x, y);
  // Figure out which bit we're interested in setting/clearing
  uint8_t bitIndex = 7-(x%8);
  // Now we'll use the set/reset bit command to toggle the bit we're
  //  interested in.
  // Of course, before we can do that, we need to determine, based on the
  //  state of reverse, whether "ON" and "OFF" correspond to set/reset or
  //  reset/set.
  if (reverse) // We're in dark-on-light mode...
  {
    if (pixel == ON) // ...so ON corresponds to a pixel that is dark.
        t6963BitSR(bitIndex, PIX_DK);
    else
        t6963BitSR(bitIndex, PIX_LT);
  }
  else
  {
    if (pixel == ON) // ...so ON corresponds to a pixel that is light.
        t6963BitSR(bitIndex, PIX_LT);
    else
        t6963BitSR(bitIndex, PIX_DK);
  }
}

// Read an 8x8 block of pixels. Pixels in the t6963 world are in 8-bit blocks,
//  so we may need to read up to 16 bytes of data and do some shifting around
//  to get the data we want. The data that we return should be a buffer of
//  columns; that is, the first byte in the buffer will be (x, y) to (x, y+7),
//  and the last buffer should be (x+7, y) to (x+7, y+7). This sucks, b/c we
//  are getting data from the display in the form (x, y) to (x+7, y) and we
//  need to effectively rotate that matrix 90 degrees, bit by bit.
void t6963ReadBlock(uint8_t x, uint8_t y, uint8_t *buffer)
{
  uint8_t firstColBuffer, secondColBuffer;
  uint8_t dataBuffer[8];
  for (uint8_t i = 0; i < 8; i++)
  {
    t6963SetPointer(x,y+i); // This sets our pointer to the location containing
                            //  the first pixel of interest.
    t6963WriteCmd(0xc1);    // Read data command, increment pointer.
    firstColBuffer = t6963ReadData(); // Read the data.
    t6963WriteCmd(0xc5);    // Read data command, don't change pointer.
    secondColBuffer = t6963ReadData(); // Read the data.
    // Okay, so now we have the data we're interested in. We'll need to
    //  bit-shift it; if the data spans two bytes, we need to put those two
    //  bytes into one.
    dataBuffer[i] = firstColBuffer<<(x%8);
    dataBuffer[i] |= secondColBuffer>>(8 - (x%8));
  }
  // dataBuffer now contains the block data, with dataBuffer[0] being the top
  //  row. We need to make buffer[0] contain bit 0 of each of dataBuffer's
  //  elements, buffer[1] contain bit 1, etc etc.
  for (uint8_t i = 0; i < 8; i++)
  {
    buffer[i] = 0; // Before we start OR-ing stuff into this, make sure the
                   //  address is clean!
    for (uint8_t j = 0; j <8; j++)
    {
      buffer[i] |= (dataBuffer[j]&(0x01<<j));
    }
  } 
}