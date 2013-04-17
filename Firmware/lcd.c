#include<avr/io.h>
#include "glcdbp.h"
#include "ks0108b.h"
#include "lcd.h"
#include "serial.h"

extern enum DISPLAY_TYPE display;

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
 
void lcdDrawLine(int8_t p1x, int8_t p1y, int8_t p2x, int8_t p2y)
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
			putChar('1');
		
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
			putChar('2');
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
			putChar('3');
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
			putChar('4');
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

void lcdDrawCircle(uint8_t x0, uint8_t y0, uint8_t r)
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

void lcdDrawPixel(uint8_t x, uint8_t y, PIX_VAL pixel)
{
	if (display == SMALL)
	{
		if (x<128 && y<64) ks0108bDrawPixel(x, y, pixel);
	}
}
