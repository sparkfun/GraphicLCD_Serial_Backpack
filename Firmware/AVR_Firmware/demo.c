/***************************************************************************
demo.c

Demo mode support file for the graphic lcd backpack. I put this in one file
 to make editing it easy and clean, and to make removing it to free up
 memory easy.

02 May 2013 - Mike Hord, SparkFun Electronics

This code is released under the Creative Commons Attribution Share-Alike 3.0
 license. You are free to reuse, remix, or redistribute it as you see fit,
 so long as you provide attribution to SparkFun Electronics.

***************************************************************************/

#include <avr/pgmspace.h> 
#include <util/delay.h>
#include "lcd.h"
#include "demo.h"
#include "serial.h"
#include "glcdbp.h"

// These variables are defined in lcd.c, and form the backbone of the pseudo
//  terminal text handling system.
extern uint8_t  cursorPos[];
extern uint8_t  textOrigin[];
extern uint16_t textLength;
extern uint8_t  yDim;
extern uint8_t  xDim;

void demo(void)
{
  char buffer[64];
  uint8_t i = 0;
  uint8_t j = 0;
  
  // We're going to start out by printing some strings that are stored in
  //  flash memory.
  for (i = 0; i<6; i++)
  {
    // Copy the string from flash into a RAM buffer.
    strcpy_P(buffer, (PGM_P)pgm_read_word(&(wantYouGone[i])));
    // Iterate over the buffer until you hit the end, drawing each character
    //  as it is encountered and delaying between lines.
    while(buffer[j] != '\0')
    {
      lcdDrawChar(buffer[j++]);
    }
    lcdDrawChar('\r');
    _delay_ms(750);
    j = 0;  // Before each line, we need to reset our string counter.
    // Also, for dramatic effect, we want to do a couple of things after
    //  specific lines.
    if ((i == 4) | (i == 3))
    {
      _delay_ms(750);
      lcdClearScreen();
      cursorPos[0] = 0;
      cursorPos[1] = (yDim/2)-8;
    }      
  }  
  
  // Okay, now we've finished that part- let's do some useful diagnostic stuff.
  //  Let's draw a sprite-based checkerboard. Sprite 127 is probably just a
  //  solid block; after all, that's what it is by default. We'll iterate over
  //  the whole screen, drawing ON and OFF sprites.
  
  uint8_t onOff = 0x01;  // Is the first square ON or OFF?
  for (j = 0; j<(yDim-7); j+=8)
  {
    for (i = 0; i<(xDim-7); i+=8)
    {
      if (onOff) lcdDrawSprite(i, j, 127, '0', ON);
      else       lcdDrawSprite(i, j, 127, '0', OFF);
      onOff ^= 0x01;
    }
    onOff ^= 0x01;
  }  
  
  _delay_ms(500);
  
  // Next, let's draw some awesome concentric circles across the screen.
  for (i = 2; i < (xDim/2); i += 8)
  {
    lcdDrawCircle(xDim/2, yDim/2, i, ON);
    _delay_ms(250);
  } 
    
  uint8_t openShut = 0x01;
  // Now we're going to send in Pac-man and a ghost to clean up.
  for (j = 0; j<(yDim-7); j+=8)
  {
    for (i = 0; i<(xDim-7); i+=8)
    {
      if (openShut) lcdDrawSprite(i, j, 4, '0', ON);
      else          lcdDrawSprite(i, j, 5, '0', ON);
      openShut ^= 0x01;
      lcdDrawSprite(i+10, j, 0, '0', ON);
      _delay_ms(200);
      lcdEraseBlock(i, j, i+7, j+7);
      lcdEraseBlock(i+10, j, i+17, j+7);
    }
  } 
}
