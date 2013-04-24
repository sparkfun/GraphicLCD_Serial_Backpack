#include<avr/io.h>
#include <avr/pgmspace.h> 
#include "glcdbp.h"
#include "lcd.h"
#include "ks0108b.h"
#include "serial.h"

extern enum DISPLAY_TYPE display;
extern uint8_t reverse;

uint8_t  cursorPos[] = {0,0};
uint8_t  textOrigin[] = {0,0};
uint16_t textLength = 0; // Number of characters typed since last time we set
                         //   cursorPos to textOrigin. Facilitates backspace.
uint8_t  xDim = 128;
uint8_t  yDim = 64;

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
}

void lcdClearScreen(void)
{
  cursorPos[0] = textOrigin[0];
  cursorPos[1] = textOrigin[1];
  textLength = 0;
  if (display == SMALL)	ks0108bClear();
}

 // Draws a line between two points p1(p1x,p1y) and p2(p2x,p2y).
 // This function is based on the Bresenham's line algorithm and is highly 
 // optimized to be able to draw lines very quickly. There is no floating point 
 // arithmetic nor multiplications and divisions involved. Only addition, 
 // subtraction and bit shifting are used. 

 // Note that you have to define your own customized lcdDrawPixel(x,y) function, 
 // which essentially lights a pixel on the screen.
 
 // This code adopted from code originally posted to codekeep.net
 //	 (http://www.codekeep.net/snippets/e39b2d9e-0843-4405-8e31-44e212ca1c45.aspx)
 //	 by Woon Khang Tang on 1/29/2009.
 
void lcdDrawLine(int8_t p1x, int8_t p1y, int8_t p2x, int8_t p2y, PIX_VAL pixel)
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

void lcdDrawBox(int8_t p1x, int8_t p1y, int8_t p2x, int8_t p2y, PIX_VAL pixel)
{
	lcdDrawLine(p1x, p1y, p1x, p2y, pixel);
	lcdDrawLine(p1x, p1y, p2x, p1y, pixel);
	lcdDrawLine(p2x, p2y, p1x, p2y, pixel);
	lcdDrawLine(p2x, p2y, p2x, p1y, pixel);
}

void lcdDrawChar(char printMe)
{
  // So, we'll check our three special cases first: backspace, newline, and
  //   carriage return.
  switch(printMe)
  {
    case '\r':
    while (cursorPos[0] <= (xDim-6)) 
    {
      cursorPos[0] += 6;
      textLength++;
    }
    cursorPos[0] = textOrigin[0];
    cursorPos[1] += 8;
    if (cursorPos[1] >= (yDim-7)) cursorPos[1] = textOrigin[1];
    break;
    
    case '\b':
    if (textLength > 0) // no text, no backspace!
    {
      textLength--;
      // special case: we're at the beginning of a line.
      if (cursorPos[0] == textOrigin[0])
      {
        // Even more special: we're at the top of the text block
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
  
	if ((printMe >= ' ') && (printMe <= '~'))
	{
		uint16_t charOffset = printMe - ' ';
		charOffset=5*charOffset;
    textLength++;
    // This section implements a rapid-write- assumes a grid of 6x8 characters.
		//   I'm leaving it in because it's cool; we need to implement an arbitrary
    //   character write function for random locations. Maybe we'll find a use
    //   for it in the future?
    /*
    for (uint8_t i = 0; i<5; i++)
		{
			uint8_t colTemp = pgm_read_byte(&characterArray[charOffset+i]);
			lcdDrawColumn(cursorPos[0]*6 + i, cursorPos[1], colTemp);
		}
		if (++cursorPos[0]>20)
		{
			cursorPos[0] = 0;
			if (++cursorPos[1]>7) cursorPos[1] = 0;
		}*/
    
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
      buffer[i-spriteIndex] &= ~pgm_read_byte(&maskArray[i]);
      buffer[i-spriteIndex] |= pgm_read_byte(&spriteArray[i]);
    }
    // The buffer now holds the block as it should look. Now we need to print
    //  it, pixel by pixel, to the screen.
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
//   be erased pixel by pixel, but we *could* do it column by column. 
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

void lcdDrawLogo(void)
{
  uint8_t leftEdge = ((xDim/2)-10);

  for (uint8_t i = 0; i<10; i++)
  {
    uint8_t colTemp = pgm_read_byte(&logoArray[i]);
    lcdDrawColumn(leftEdge+i, 3, colTemp);
  }
  for (uint8_t i = 10; i<20; i++)
  {
    uint8_t colTemp = pgm_read_byte(&logoArray[i]);
    lcdDrawColumn(leftEdge-10+i, 4, colTemp);
  }
}

void lcdDrawColumn(uint8_t x, uint8_t y, uint8_t colVal)
{
	if (display == SMALL)
	{
		if (x<xDim) ks0108bDrawColumn(x, y, colVal);
	}
}

void lcdDrawPixel(uint8_t x, uint8_t y, PIX_VAL pixel)
{
	if (display == SMALL)
	{
		if (x<xDim && y<yDim) ks0108bDrawPixel(x, y, pixel);
	}
}

void lcdGetDataBlock(uint8_t x, uint8_t y, uint8_t *buffer)
{
  if (display == SMALL) ks0108bReadBlock(x, y, buffer);
}
