#include <avr/io.h>
#include <util/delay.h>		// F_CPU is defined in the makefile
#include "glcdbp.h"
#include "serial.h"
#include "io_support.h"
#include "t6963.h"

extern volatile uint8_t reverse;

void t6963Clear(void)
{
  t6963SetPointer(0,0);
  //t6963WriteCmd(0xb0);
  for (uint16_t i = 0; i < 2560; i++)
  {
    //while ((t6963ReadStatus() & 0x04) == 0x00);
    if (reverse) t6963WriteData(0xff);
    else         t6963WriteData(0x00);
    t6963WriteCmd(0xc0);
  }
  //while ((t6963ReadStatus() & 0x04) == 0x00);
  //t6963WriteCmd(0xb2);
}

// Write a data byte to the controller. Data lines 7:2 are connected to pins
//  B7:2 and 1:0 are on D1:0.
void t6963WriteData(uint8_t data)
{
  t6963BusyWait();
  setData(data);
  PORTC &= ~(1<<CD);
  _delay_us(1);
  PORTC &= ~((1<<WR) |
             (1<<CE));
  _delay_us(1);
  PORTC |= (1<<CE);
  PORTC |= ((1<<CD) |
            (1<<WR) |
            (1<<RD));
}

uint8_t t6963ReadData(void)
{  
  t6963BusyWait();
  uint8_t data;
  PORTC &= ~(1<<CD);
  _delay_us(1);
  PORTC &= ~( (1<<CE) |
              (1<<RD) );
  _delay_us(1);
  data = readData();
  PORTC |= (1<<CE);
  PORTC |= ((1<<CD) |
            (1<<WR) |
            (1<<RD));
  return data;
}

void t6963WriteCmd(uint8_t command)
{  
  t6963BusyWait();
  setData(command);
  _delay_us(1);
  PORTC &= ~(1<<WR);
  _delay_us(1);
  PORTC &= ~(1<<CE);
  _delay_us(1);
  PORTC |= (1<<CE);
  PORTC |= ((1<<CD) |
            (1<<WR) |
            (1<<RD));
}

uint8_t t6963ReadStatus(void)
{  
  uint8_t status;
  PORTC &= ~(1<<RD);
  _delay_us(1);
  PORTC &= ~(1<<CE);
  _delay_us(1);
  status = readData();
  PORTC |= (1<<CE);
  PORTC |= ((1<<CD) |
            (1<<WR) |
            (1<<RD));
  return status;
}

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
  t6963BusyWait();
  t6963WriteCmd(command);
  /*
  // Let's try doing the old-fashioned read-write method.
  // Start by issuing the "read-but-don't-change-pointer" command.
  t6963WriteCmd(0xc5);
  // Now, when we do a readData command, we should get the data at the pointer
  //  address.
  uint8_t dataBuffer = t6963ReadData();
  putBin(dataBuffer);
  putLine(" ");
  // Create a bit mask for the setting/clearing of the appropriate bit.
  uint8_t bitMask = (0x01)<<bit;
  // Do we want to turn the pixel off, or on?
  if (SR == PIX_DK) dataBuffer &= ~(bitMask); // Turn pixel OFF
  else              dataBuffer |= bitMask;    // Turn pixel ON
  // Write the data to the device...
  t6963WriteData(dataBuffer);
  // ...and tell the device to write that data to the current pointer, no
  //  increment of the pointer.
  t6963WriteCmd(0xc4);*/
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
