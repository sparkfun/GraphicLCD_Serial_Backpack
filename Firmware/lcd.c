#include<avr/io.h>
#include <avr/pgmspace.h> 
#include "lcd.h"
#include "ks0108b.h"
#include "serial.h"

extern enum DISPLAY_TYPE display;

uint8_t cursorPos[] = {0,0};

void lcdConfig(void)
{
	ks0108bReset();
	ks0108bDisplayOn();
	ks0108bClear();
}

void lcdClearScreen(void)
{
	ks0108bClear();
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
	if (printMe >= ' ' && printMe <= '~')
	{
		uint16_t charOffset = printMe - ' ';
		charOffset=5*charOffset;
		for (uint8_t i = 0; i<5; i++)
		{
			uint8_t colTemp = pgm_read_byte(&characterArray[charOffset+i]);
			lcdDrawColumn(cursorPos[0]*6 + i, cursorPos[1], colTemp);
		}
		if (++cursorPos[0]>21)
		{
			cursorPos[0] = 0;
			if (++cursorPos[1]>7) cursorPos[1] = 0;
		}
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
  if (display == SMALL)
  {
    for (uint8_t i = 0; i<10; i++)
    {
      uint8_t colTemp = pgm_read_byte(&logoArray[i]);
      lcdDrawColumn(54+i, 3, colTemp);
    }
    for (uint8_t i = 10; i<20; i++)
    {
      uint8_t colTemp = pgm_read_byte(&logoArray[i]);
      lcdDrawColumn(44+i, 4, colTemp);
    }
  }
}

void lcdDrawColumn(uint8_t x, uint8_t y, uint8_t colVal)
{
	if (display == SMALL)
	{
		if (x<128) ks0108bDrawColumn(x, y, colVal);
	}
}

void lcdDrawPixel(uint8_t x, uint8_t y, PIX_VAL pixel)
{
	if (display == SMALL)
	{
		if (x<128 && y<64) ks0108bDrawPixel(x, y, pixel);
	}
}
