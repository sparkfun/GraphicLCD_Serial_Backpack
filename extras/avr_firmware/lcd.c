/***************************************************************************
lcd.c

LCD hardware support file for the serial graphical LCD backpack project.
 This is the API for the LCD; external modules should call the functions
 provided here rather than calling the device drivers directly. Provides for
 font rendering, sprite rendering, drawing lines, circles, points and boxes,
 and erasing blocks of arbitrary size.

02 May 2013 - Mike Hord, SparkFun Electronics

This code is released under the Creative Commons Attribution Share-Alike 3.0
 license. You are free to reuse, remix, or redistribute it as you see fit,
 so long as you provide attribution to SparkFun Electronics.

***************************************************************************/

#include <avr/io.h>
#include <avr/pgmspace.h> 
#include "glcdbp.h"
#include "lcd.h"
#include "ks0108b.h"
#include "serial.h"
#include "t6963.h"

// These variables are defined in glcdbp.c, and allow us to take actions based
//  on the type of display and the operating mode (reverse or normal).
extern enum DISPLAY_TYPE display;
extern volatile uint8_t reverse;

// These values allow us to emulate a terminal of arbitrary size. cursorPos is
//  the x,y pixel of the upper left corner of where the current pixel will be
//  drawn; textOrigin is the upper left corner of the region of the screen that
//  text will be drawn within. The right and bottom edges of that region are
//  bounded by the edge of the display.
uint8_t  cursorPos[] = {0,0};
uint8_t  textOrigin[] = {0,0};
uint16_t textLength = 0; // Number of characters typed since last time we set
                         //  cursorPos to textOrigin. Facilitates backspace.

// Because we have two different types of display, it's nice to be able to 
//  not hard-code the dimensions in cases where we may want to set limits or
//  find the center of the screen.
uint8_t  xDim = 128;
uint8_t  yDim = 64;

// Configure functions for the two display types. The details are in the
//  appropriate driver files.
void lcdConfig(void)
{
  if (display == SMALL)
  {
    ks0108bReset();
    ks0108bDisplayOn();
    ks0108bClear();
    xDim = 128;
    yDim = 64;
  }
  else
  {
    t6963DisplayInit();
    xDim = 160;
    yDim = 128;
  }
}

// Reset our text mode, then call the driver specific clear screen command.
void lcdClearScreen(void)
{
  cursorPos[0] = textOrigin[0];
  cursorPos[1] = textOrigin[1];
  textLength = 0;
  if (display == SMALL)	ks0108bClear();
  else t6963Clear();
}

 // Draws a line between two points p1(p1x,p1y) and p2(p2x,p2y).
 // This function is based on the Bresenham's line algorithm and is highly 
 // optimized to be able to draw lines very quickly. There is no floating point 
 // arithmetic nor multiplications nor divisions involved. Only addition, 
 // subtraction and bit shifting are used. 

 // Note that you have to define your own customized lcdDrawPixel(x,y) function, 
 // which essentially lights a pixel on the screen.
 
 // This code adopted from code originally posted to codekeep.net
 //	 (http://www.codekeep.net/snippets/e39b2d9e-0843-4405-8e31-44e212ca1c45.aspx)
 //	 by Woon Khang Tang on 1/29/2009.
 
void lcdDrawLine(uint8_t p1x, uint8_t p1y, uint8_t p2x, uint8_t p2y, PIX_VAL pixel)
{
    int16_t F, x, y;

    if (p1x > p2x)  // Swap points if p1 is on the right of p2
    {
      x = p1x;
      p1x = p2x;
      p2x = x;
      y = p1y;
      p1y = p2y;
      p2y = y;
    }

    // Handle trivial cases separately for algorithm speed up.
    // Trivial case 1: m = +/-INF (Vertical line)
    if (p1x == p2x)
    {
        if (p1y > p2y)  // Swap y-coordinates if p1 is above p2
        {
          y = p1y;
          p1y = p2y;
          p2y = y;
        }
        x = p1x;
        y = p1y;
        while (y <= p2y)
        {
          lcdDrawPixel(x, y, ON);
          y++;
        }
        return;
    }
    // Trivial case 2: m = 0 (Horizontal line)
    else if (p1y == p2y)
    {
        x = p1x;
        y = p1y;

        while (x <= p2x)
        {
          lcdDrawPixel(x, y, ON);
          x++;
        }
        return;
    }

    int16_t dy            = p2y - p1y;  // y-increment from p1 to p2
    int16_t dx            = p2x - p1x;  // x-increment from p1 to p2
    int16_t dy2           = (dy << 1);  // dy << 1 == 2*dy
    int16_t dx2           = (dx << 1);
    int16_t dy2_minus_dx2 = dy2 - dx2;  // precompute constant for speed up
    int16_t dy2_plus_dx2  = dy2 + dx2;

    if (dy >= 0)    // m >= 0
    {
        // Case 1: 0 <= m <= 1 (Original case)
        if (dy <= dx)   
        {
		
            F = dy2 - dx;    // initial F

            x = p1x;
            y = p1y;
            while (x <= p2x)
            {
                lcdDrawPixel(x, y, ON);
                if (F <= 0)
                {
                    F += dy2;
                }
                else
                {
                    y++;
                    F += dy2_minus_dx2;
                }
                x++;
            }
        }
        // Case 2: 1 < m < INF (Mirror about y=x line
        // replace all dy by dx and dx by dy)
        else
        {
            F = dx2 - dy;    // initial F

            y = p1y;
            x = p1x;
            while (y <= p2y)
            {
                lcdDrawPixel(x, y, ON);
                if (F <= 0)
                {
                    F += dx2;
                }
                else
                {
                    x++;
                    F -= dy2_minus_dx2;
                }
                y++;
            }
        }
    }
    else    // m < 0
    {
        // Case 3: -1 <= m < 0 (Mirror about x-axis, replace all dy by -dy)
        if (dx >= -dy)
        {
            F = -dy2 - dx;    // initial F

            x = p1x;
            y = p1y;
            while (x <= p2x)
            {
                lcdDrawPixel(x, y, ON);
                if (F <= 0)
                {
                    F -= dy2;
                }
                else
                {
                    y--;
                    F -= dy2_plus_dx2;
                }
                x++;
            }
        }
        // Case 4: -INF < m < -1 (Mirror about x-axis and mirror 
        // about y=x line, replace all dx by -dy and dy by dx)
        else    
        {
            F = dx2 + dy;    // initial F

            y = p1y;
            x = p1x;
            while (y >= p2y)
            {
                lcdDrawPixel(x, y, ON);
                if (F <= 0)
                {
                    F += dx2;
                }
                else
                {
                    x++;
                    F += dy2_plus_dx2;
                }
                y--;
            }
        }
    }
}

// I found this code on wikipedia- it's the general circle version of
//  Bresenham's line algorithm. It works great. I'm not going to attempt to
//  comment it- look it up yourself, lazy.
void lcdDrawCircle(uint8_t x0, uint8_t y0, uint8_t r, PIX_VAL pixel)
{
  int x = r, y = 0;
  int xChange = 1 - (r << 1);
  int yChange = 0;
  int radiusError = 0;
 
  while(x >= y)
  {
    lcdDrawPixel(x + x0, y + y0, ON);
    lcdDrawPixel(y + x0, x + y0, ON);
    lcdDrawPixel(-x + x0, y + y0, ON);
    lcdDrawPixel(-y + x0, x + y0, ON);
    lcdDrawPixel(-x + x0, -y + y0, ON);
    lcdDrawPixel(-y + x0, -x + y0, ON);
    lcdDrawPixel(x + x0, -y + y0, ON);
    lcdDrawPixel(y + x0, -x + y0, ON);
 
    y++;
    radiusError += yChange;
    yChange += 2;
    if(((radiusError << 1) + xChange) > 0)
    {
      x--;
      radiusError += xChange;
      xChange += 2;
    }
  }
}

// Draw box is just four lines. It's really just a shortcut.
void lcdDrawBox(uint8_t p1x, uint8_t p1y, uint8_t p2x, uint8_t p2y, PIX_VAL pixel)
{
	lcdDrawLine(p1x, p1y, p1x, p2y, pixel);
	lcdDrawLine(p1x, p1y, p2x, p1y, pixel);
	lcdDrawLine(p2x, p2y, p1x, p2y, pixel);
	lcdDrawLine(p2x, p2y, p2x, p1y, pixel);
}

// This is the by-pixel character rendering function. At this point, there's no
//  support for the t6963 built-in character generator. Get on that, won't you?
void lcdDrawChar(char printMe)
{
  // So, we'll check our three special cases first: backspace and newline.
  switch(printMe)
  {
    case '\r':  // Newline.
    // For backspace tracking purposes, we want to track how many characters
    //  we're skipping on this line.
    while (cursorPos[0] <= (xDim-6))
    {
      cursorPos[0] += 6;
      textLength++;
    }
    // Then, we want to reset the imaginary cursor to the start of the next
    //  "line" of text- 8 pixels below the top of the current line.
    cursorPos[0] = textOrigin[0];
    cursorPos[1] += 8;
    // If we've reached the bottom of the screen, we want to wrap to the top
    //  of the area that we defined to contain text by setting the text origin
    //  at some earlier time.
    if (cursorPos[1] >= (yDim-7)) cursorPos[1] = textOrigin[1];
    break;
    
    case '\b':
    if (textLength > 0) // no text, no backspace!
    {
      textLength--; // Reduce the number of characters we've written.
      
      // Now, we'll move our cursor to the position of the last character we
      //  printed, so we can delete it.
      
      // Special case: we're at the beginning of a line.
      //  We'll want to go to the end of the previous line and delete what
      //  we find there.
      if (cursorPos[0] == textOrigin[0])
      {
        // Even more special: we're at the top of the text block, so we want
        //  to go to the bottom line of the text block, last character, and
        //  delete what we find there.
        if (cursorPos[1] == textOrigin[1])
        {
          while (cursorPos[1] < (yDim-8)) cursorPos[1] +=8;
          while (cursorPos[0] <= (xDim-6)) cursorPos[0] += 6;
          cursorPos[0]-=6;
        }
        else // Not at the top of the block, just the start of the line.
        {
          cursorPos[1] -= 8;
          while (cursorPos[0] <= (xDim-6)) cursorPos[0] += 6;
          cursorPos[0]-=6;
        }
      }
      // Normal case: not at the left or top edge of the block
      else
      {
        cursorPos[0] -= 6;
      } 
      
      // Now that our cursor is where it ought to be, we can blank out the
      //   current character location by turning the pixels there off.
      for (uint8_t x = cursorPos[0]; x<cursorPos[0]+5; x++)
      {
        for (uint8_t y = cursorPos[1]; y<cursorPos[1]+8; y++)
        {
          lcdDrawPixel(x,y,OFF);
        }
      }
    }
    break;
  }
  
  // Special cases handled; we can move on to the general case of characters
  //  that we can print out. That's everything between space and tilde.
	if ((printMe >= ' ') && (printMe <= '~'))
	{
    // All the characters are stored in a big huge block of the flash memory;
    //  each one takes five bytes. To find the five bytes in question, we need
    //  to subtract the offset of ASCII values we can't print (everything
    //  before ' ') and then multiply by five.
		uint16_t charOffset = printMe - ' ';
		charOffset=5*charOffset;
    textLength++;
    
    // This is the arbitrary character generator. For this, cursorPos is the
    //   upper left of the character, and we'll draw it pixel by pixel, one
    //   column at a time. It's slower, but more flexible.
    for (uint8_t x = cursorPos[0]; x<cursorPos[0]+5; x++)
    {
      uint8_t colTemp = pgm_read_byte(&characterArray[charOffset++]);
      for (uint8_t y = cursorPos[1]; y<cursorPos[1]+8; y++)
      {
        if ((colTemp>>(y-cursorPos[1]))&0x01) lcdDrawPixel(x,y,ON);
        else lcdDrawPixel(x,y,OFF);
      }
    }
    cursorPos[0] += 6;  // Increment our x position by one character space.
    // if we're at the end of the line, we need to wrap to the next line.
    if (cursorPos[0] >= (xDim-6))
    {
      cursorPos[0] = textOrigin[0];
      cursorPos[1] += 8;
      if (cursorPos[1] >= (yDim-7)) cursorPos[1] = textOrigin[1];
    }
	}	
}

// Sprite drawing is just like character drawing, except for two things:
//  the size (a sprite is 8x8 pixels, instead of 6x8), and sprites use a
//  mask to preserve some portion of the pixels in the region as they were
//  before the write occurred. As with characters, x and y are the upper left
//  corner of the sprite. angle is establised by one of four ASCII characters-
//  '0', '3', '6', '9', which correspond to hands on a clock.
void lcdDrawSprite(uint8_t x, uint8_t y, uint8_t sprite, char angle, 
                    PIX_VAL pixel)
  {
    uint16_t spriteIndex = sprite*8; // Index of the first byte of our sprite.
    uint8_t buffer[8]; // We'll use this buffer, along with the mask, to
                       //  draw only the sprite without disturbing the
                       //  background.
    lcdGetDataBlock(x, y, buffer);
    // Okay, now our buffer is full of the contents of the landing zone for
    //  the sprite. We can now clear the sprite's landing spot (by ANDing with
    //  the mask) and draw in bits where the sprite should be (by ORing with
    //  the sprite). To accommodate reverse mode, we'll complement all the bits
    //  before masking. The draw routine will handle the rendering.
    for (uint16_t i = spriteIndex; i < spriteIndex + 8; i++)
    {
      if (reverse) buffer[i-spriteIndex] ^= 0xff;
      buffer[i-spriteIndex] &= pgm_read_byte(&maskArray[i]);
      buffer[i-spriteIndex] |= pgm_read_byte(&spriteArray[i]);
    }
    // The buffer now holds the block as it should look. Now we need to print
    //  it, pixel by pixel, to the screen. I'm not going to go into the
    //  nitty gritty details of how I figured this out- suffice it to say,
    //  graph paper was involved.
    switch(angle)
    {
      case '0':
      for (uint8_t i = 0; i<8; i++)
      {
        for (uint8_t j = 0; j<8; j++)
        {
          if (pixel == ON)
          {
            if (buffer[i]&0x01) lcdDrawPixel(x+i,y+7-j,ON);
            else lcdDrawPixel(x+i,y+7-j,OFF);
          }
          else
          {
            if (buffer[i]&0x01) lcdDrawPixel(x+i,y+7-j,OFF);
            else lcdDrawPixel(x+i,y+7-j,ON);
          }
          buffer[i] = buffer[i]>>1;
        }
      }
      break;
      case '3':
      for (uint8_t i = 0; i<8; i++)
      {
        for (uint8_t j = 0; j<8; j++)
        {
          if (pixel == ON)
          {
            if (buffer[i]&0x01) lcdDrawPixel(x+j,y+i,ON);
            else lcdDrawPixel(x+j,y+i,OFF);
          }
          else
          {
            if (buffer[i]&0x01) lcdDrawPixel(x+j,y+i,OFF);
            else lcdDrawPixel(x+j,y+i,ON);
          }
          buffer[i] = buffer[i]>>1;
        }
      }
      break;
      case '6':
      for (uint8_t i = 0; i<8; i++)
      {
        for (uint8_t j = 0; j<8; j++)
        {
          if (pixel == ON)
          {
            if (buffer[i]&0x01) lcdDrawPixel(x+7-i,y+j,ON);
            else lcdDrawPixel(x+7-i,y+j,OFF);
          }
          else
          {
            if (buffer[i]&0x01) lcdDrawPixel(x+7-i,y+j,OFF);
            else lcdDrawPixel(x+7-i,y+j,ON);
          }
          buffer[i] = buffer[i]>>1;
        }
      }
      break;
      case '9':
      for (uint8_t i = 0; i<8; i++)
      {
        for (uint8_t j = 0; j<8; j++)
        {
          if (pixel == ON)
          {
            if (buffer[i]&0x01) lcdDrawPixel(x+7-j,y+7-i,ON);
            else lcdDrawPixel(x+7-j,y+7-i,OFF);
          }
          else
          {
            if (buffer[i]&0x01) lcdDrawPixel(x+7-j,y+7-i,OFF);
            else lcdDrawPixel(x+7-j,y+7-i,ON);
          }
          buffer[i] = buffer[i]>>1;
        }
      }
      break;
    }
  }

// This function has room for lots of improvement. We draw over the block to
//   be erased pixel by pixel, but we *could* do it column by column on the
//   ks0108b or row by row on the t6963.
void lcdEraseBlock(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
  // We want to go from upper left to lower right- if some degenerate user
  //   thinks to be cute, we want to make sure that our points are redefined
  //   so that (x0,y0) is the upper left, and (x1,y1) is lower right.
  if (x1<x0)
  {
    uint8_t xTemp = x0;
    x0 = x1;
    x1 = xTemp;
  }
  if (y1<y0)
  {
    uint8_t yTemp = y0;
    y0 = y1;
    y1 = yTemp;
  }
  // Now that we've got that settled, we'll start at point (x0,y0) and just
  //   use the lcdPixelDraw() function to turn off each pixel in the block.
  //   We'll do this by defining points (i,j), iterating across in x, then
  //   down y.
  for (uint8_t j = y0; j <= y1; j++)
  {
    for (uint8_t i = x0; i <= x1; i++)
    {
      lcdDrawPixel(i,j,OFF);
    }
  }
}

// Draw the SparkFun logo, pixel by pixel. We do this as a splash screen.
void lcdDrawLogo(void)
{
  // x and y are the left and top edges of the logo. We want to center the
  //  logo; it's 16 pixels tall and 20 pixels wide. Thus, the center of the
  //  screen is 10 pixels to far to the right and 8 pixels to far down.
  uint8_t x = ((xDim/2)-10);
  uint8_t y  = ((yDim/2)-8);

  // Now we're going to iterate over the bytes in the logo array, starting
  //  with the top half. The logo is stored with the 10 bytes forming the
  //  top half first in memory, then the 10 for the bottom half next.
  for (uint8_t i = 0; i<10; i++)
  {
    // Pull in the next byte of the logo.
    uint8_t colTemp = pgm_read_byte(&logoArray[i]);
    // Iterate over the bits in that byte, drawing one pixel at a time by
    //  looking at the lowest bit in the byte, and drawing according to
    //  that pixel and the 'reverse' flag.
    for (uint8_t j = 0; j<8; j++)
    {
      if (reverse == 0)
      {
        if (colTemp&0x01) lcdDrawPixel(x+i,y+j,ON);
        else lcdDrawPixel(x+i,y+j,OFF);
      }
      else
      {
        if (colTemp&0x01) lcdDrawPixel(x+i,y+j,OFF);
        else lcdDrawPixel(x+i,y+j,ON);
      }
      // By right shifting, we keep the LSb the bit we're interested in.
      //  That way, we only need to mask it with 0x01 and get a zero/nonzero
      //  reading on it.
      colTemp = colTemp>>1;
    }
  }  
  y+=8;  // Increment y, since we're drawing the lower half of the logo. Then
         //  we do more or less exactly the same thing.
  for (uint8_t i = 10; i<20; i++)
  {
    uint8_t colTemp = pgm_read_byte(&logoArray[i]);
    for (uint8_t j = 0; j<8; j++)
    {
      if (reverse == 0)
      {
        if (colTemp&0x01) lcdDrawPixel(x+i-10,y+j,ON);
        else lcdDrawPixel(x+i-10,y+j,OFF);
      }
      else
      {
        if (colTemp&0x01) lcdDrawPixel(x+i-10,y+j,OFF);
        else lcdDrawPixel(x+i-10,y+j,ON);
      }
      colTemp = colTemp>>1;
    }
  }

}

// lcdDrawPixel() is the generic front end to the display-specific drawPixel
//  commands. We gate the draw to save time- no point in drawing a pixel that
//  is outside the display area, which can happen in the case of large
//  circles or lines or boxes.
void lcdDrawPixel(uint8_t x, uint8_t y, PIX_VAL pixel)
{
	if (display == SMALL)
	{
		if (x<xDim && y<yDim) ks0108bDrawPixel(x, y, pixel);
	}
  else if (display == LARGE)
  {
    if (x<xDim && y<yDim) t6963DrawPixel(x, y, pixel);
  }
}

// Front-end for the display specific readBlock functions. This gets used in
//  the draw sprite function to allow sprites to be drawn over the existing
//  background. The data comes back as a block of 8 bytes; bit 0 is the upper
//  pixel; byte 0 is the leftmost column. Both types of display use this
//  structure but it's easier for the ks0108b.
void lcdGetDataBlock(uint8_t x, uint8_t y, uint8_t *buffer)
{
  if (display == SMALL) ks0108bReadBlock(x, y, buffer);
  else                  t6963ReadBlock(x, y, buffer);
}
