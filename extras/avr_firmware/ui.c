/***************************************************************************
ui.c

Handles all the user interface parsing when data comes in over the serial
 port.

02 May 2013 - Mike Hord, SparkFun Electronics

This code is released under the Creative Commons Attribution Share-Alike 3.0
 license. You are free to reuse, remix, or redistribute it as you see fit,
 so long as you provide attribution to SparkFun Electronics.

***************************************************************************/

#include "ui.h"
#include "lcd.h"
#include "serial.h"
#include "glcdbp.h"
#include "nvm.h"
#include "demo.h"

// These variables are defined in glcdbp.c, and are used for the input buffer
//   from the serial port. We need to be able to access them here because we'll
//   be stuck here parsing input from the serial port once a command comes in.
extern volatile uint8_t 	rxRingBuffer[BUF_DEPTH];
extern volatile uint16_t 	rxRingHead;
extern volatile uint16_t	rxRingTail;
extern volatile uint8_t	  bufferSize;
extern volatile uint8_t   reverse;

// These variables are defined in lcd.c, and form the backbone of the pseudo
//  terminal text handling system.
extern uint8_t  cursorPos[];
extern uint8_t  textOrigin[];
extern uint16_t textLength;
extern uint8_t  yDim;
extern uint8_t  xDim;


// This is a state machine that acts based on the received command from the
//  main program loop.
void uiStateMachine(char command)
{
  // Up to five characters may be needed to describe any single operation.
  char cmdBuffer[5];
  // We'll want to track how far we've moved through our buffered command
  //  bytes once we've received them all.
  uint8_t cmdBufferPtr = 0;
  // We will *assume* a state of ON for pixel values; we can clear that later.
  PIX_VAL pixel = ON;
  // Now, let's figure out where to start, based on our command input. There's
  //  a lot of good information about the switch constants in the header file.

  switch(command)
  {
    case CLEAR_SCREEN:  // Easy and fairly self-explanatory.
    lcdClearScreen();
    break;
    
    case RUN_DEMO:      // Some sort of wonky song-and-dance to show off.
    lcdClearScreen();
    demo();
    reverse ^= 0x01;
    lcdClearScreen();
    demo();
    reverse ^= 0x01;
    lcdClearScreen();
    break;
    
    case TOGGLE_BGND:   // Switch between reverse mode and normal mode.
      reverse ^= 0x01;
      toggleReverse();
      lcdClearScreen();
    break;
    
    case TOGGLE_SPLASH: // Turn the splash-on-startup SparkFun logo off.
      toggleSplash();
    break;
    
    case ADJ_BL_LEVEL:  // The first real, meaty command. Adjust the backlight.
      while(1)  // Stay here until we are *told* to leave.
      {
        // If there's data in the serial buffer, move it to the cmdBuffer.
        if (bufferSize > 0)
        {
          cmdBuffer[cmdBufferPtr++] = serialBufferPop();
        }
        // This command is a one-byte command; once the count of bytes in the
        //  command buffer is greater than 0, we want to parse the command.
        if (cmdBufferPtr > 0)
        {
          cmdBufferPtr = 0;  // Reset the command buffer pointer.
          // We need to make sure our level never exceeds 100, or weird
          //   things can happen to the PWM generator.
          if ((uint8_t)cmdBuffer[0] > 100) cmdBuffer[0] = 100;
          // Set the backlight level- this is an alias to the actual register,
          //  renamed for convenience.
          BL_LEVEL = cmdBuffer[0];
          // Store the new value in EEPROM.
          setBacklightLevel(cmdBuffer[0]);
          break; // This is where we tell to code to leave the while loop.
        }
      }
    break;
    
    case ADJ_BAUD_RATE:
      while(1)  // Stay here until we are *told* to leave.
      {
        if (bufferSize > 0)
        {
          cmdBuffer[cmdBufferPtr++] = serialBufferPop();
        }
        // Again, a one-byte command. Once we have a byte in the cmdBuffer,
        //  parse it.
        if (cmdBufferPtr > 0)
        {
          cmdBufferPtr = 0;
          setBaudRate(cmdBuffer[0]); // This will reject invalid settings,
                                     //   which is to say, anything outside
                                     //   of the range ASCII 1-6.
          switch(cmdBuffer[0])
          {
            case '1':
            serialInit(BR4800);
            break;
            case '2':
            serialInit(BR9600);
            break;
            case '3':
            serialInit(BR19200);
            break;
            case '4':
            serialInit(BR38400);
            break;
            case '5':
            serialInit(BR57600);
            break;
            case '6':
            serialInit(BR115200);
            break;
            default: // If we have an invalid entry, we'll just ignore it.
            break;
          }
          break; // This is where we tell to code to leave the while loop.
        }
      }
    break;
    
    case ADJ_TEXT_X: // This is the x-origin of our text "window".
      while(1)  // Stay here until we are *told* to leave.
      {
        if (bufferSize > 0)
        {
          cmdBuffer[cmdBufferPtr++] = serialBufferPop();
        }
        // Again, one-byte command.
        if (cmdBufferPtr > 0)
        {
          cmdBufferPtr = 0;
          // The most recent byte is the new X origin of the text window- IF it
          //   makes sense. It only makes sense if it is at least 6 pixels from
          //   the right edge of the screen. We use the xDim variable from
          //   lcd.c to make sure we don't botch that. Ignore invalid input.
          if (cmdBuffer[0] <= (xDim-6))
          {
            textOrigin[0] = cmdBuffer[0];
            cursorPos[0] = textOrigin[0];
            textLength = 0;
          }
          break; // This is where we tell to code to leave the while loop.
        }
      }
    break;
    
    case ADJ_TEXT_Y:
      while(1)  // Stay here until we are *told* to leave.
      {
        if (bufferSize > 0)
        {
          cmdBuffer[cmdBufferPtr++] = serialBufferPop();
        }
        // One byte command.
        if (cmdBufferPtr > 0)
        {
          cmdBufferPtr = 0;
          // The most recent byte is the new Y origin of the text window- IF it
          //   makes sense. It only makes sense if it is at least 8 pixels from
          //   the bottom edge of the screen. Use the yDim variable from lcd.c
          //   to make sure we don't botch that. Ignore invalid input.
          if (cmdBuffer[0] <= (yDim-8))
          {
            textOrigin[1] = cmdBuffer[0];
            cursorPos[1] = textOrigin[1];
            textLength = 0;
          }
          break; // This is where we tell to code to leave the while loop.
        }
      }
    break;
    
    case DRAW_PIXEL:
      while(1)  // Stay here until we are *told* to leave.
      {
        if (bufferSize > 0)
        {
          cmdBuffer[cmdBufferPtr++] = serialBufferPop();
        }
        // This is a three-value command- x, y, ON/OFF.
        if (cmdBufferPtr > 2)
        {
          cmdBufferPtr = 0;
          // If the user *specifically* sends a 0 for the third byte, turn
          //  the pixel off. Otherwise, turn it on.
          if (cmdBuffer[2] == 0) pixel = OFF;
          lcdDrawPixel(cmdBuffer[0], cmdBuffer[1], pixel);
          break; // This is where we tell to code to leave the while loop.
        }
      }
    break;
    
    case DRAW_LINE:      
    while(1)  // Stay here until we are *told* to leave.
      {
        if (bufferSize > 0)
        {
          cmdBuffer[cmdBufferPtr++] = serialBufferPop();
        }
        // Five-byte command.
        if (cmdBufferPtr > 4)
        {
          cmdBufferPtr = 0;
          // Same sort of logic- if the user sends a 0 for the pixel value,
          //  turn pixels off, otherwise, turn them on.
          if (cmdBuffer[4] == 0) pixel = OFF;
          lcdDrawLine(cmdBuffer[0], cmdBuffer[1], // start point x,y
                      cmdBuffer[2], cmdBuffer[3], // end point x,y
                      pixel);                     // draw or erase?
          break; // This is where we tell to code to leave the while loop.
        }
      }
    
    break;
    
    case DRAW_CIRCLE:      
    while(1)  // Stay here until we are *told* to leave.
      {
        if (bufferSize > 0)
        {
          cmdBuffer[cmdBufferPtr++] = serialBufferPop();
        }
        // Four-byte command.
        if (cmdBufferPtr > 3)
        {
          cmdBufferPtr = 0;
          if (cmdBuffer[3] == 0) pixel = OFF;
          lcdDrawCircle(cmdBuffer[0], cmdBuffer[1], // center point x,y
                        cmdBuffer[2],               // radius
                        pixel);                     // draw or erase?
          break; // This is where we tell to code to leave the while loop.
        }
      }
 
    break;
    
    case DRAW_BOX:    
    while(1)  // Stay here until we are *told* to leave.
      {
        if (bufferSize > 0)
        {
          cmdBuffer[cmdBufferPtr++] = serialBufferPop();
        }
        // Five-byte command.
        if (cmdBufferPtr > 4)
        {
          cmdBufferPtr = 0;
          if (cmdBuffer[4] == 0) pixel = OFF;
          lcdDrawBox(cmdBuffer[0], cmdBuffer[1], // start point x,y
                     cmdBuffer[2], cmdBuffer[3], // end point x,y
                     pixel);                     // draw or erase?
          break; // This is where we tell to code to leave the while loop.
        }
      }
    
    break;
    
    case ERASE_BLOCK:    
    while(1)  // Stay here until we are *told* to leave.
      {
        if (bufferSize > 0)
        {
          cmdBuffer[cmdBufferPtr++] = serialBufferPop();
        }
        // Four-byte command.
        if (cmdBufferPtr > 3)
        {
          cmdBufferPtr = 0;
          lcdEraseBlock(cmdBuffer[0], cmdBuffer[1], // start point x,y
                        cmdBuffer[2], cmdBuffer[3]); // end point x,y
          break; // This is where we tell to code to leave the while loop.
        }
      }
    break;    
    
    case DRAW_SPRITE:      
    while(1)  // Stay here until we are *told* to leave.
      {
        if (bufferSize > 0)
        {
          cmdBuffer[cmdBufferPtr++] = serialBufferPop();
        }
        // Five-byte command.
        if (cmdBufferPtr > 4)
        {
          cmdBufferPtr = 0;
          if (cmdBuffer[4] == 0) pixel = OFF;
          lcdDrawSprite(cmdBuffer[0], cmdBuffer[1], // upper left x,y
                        cmdBuffer[2],               // sprite index
                        cmdBuffer[3],               // rotation angle
                        pixel);                     // draw or erase?
          break; // This is where we tell to code to leave the while loop.
        }
      }
 
    break;
    
    default: // if the character that followed the '|' is not a valid command,
    break;   //  ignore it.
  }
}
