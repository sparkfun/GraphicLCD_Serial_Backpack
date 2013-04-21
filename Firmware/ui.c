#include "ui.h"
#include "lcd.h"
#include "serial.h"
#include "glcdbp.h"
#include "nvm.h"

// These variables are defined in glcdbp.c, and are used for the input buffer
//   from the serial port. We need to be able to access them here because we'll
//   be stuck here parsing input from the serial port once a command comes in.
extern volatile uint8_t 	rxRingBuffer[BUF_DEPTH];
extern volatile uint16_t 	rxRingHead;
extern volatile uint16_t	rxRingTail;
extern volatile uint16_t	bufferSize;
extern volatile uint8_t   reverse;

extern uint8_t  cursorPos[];
extern uint8_t  textOrigin[];
extern uint16_t textLength;

void uiStateMachine(char command)
{
  char cmdBuffer[5];
  uint8_t cmdBufferPtr = 0;
  PIX_VAL pixel = ON;
  switch(command)
  {
    case CLEAR_SCREEN:
    lcdClearScreen();
    break;
    
    case RUN_DEMO:
    // to be implemented
    break;
    
    case TOGGLE_BGND:
      reverse ^= 0x01;
      toggleReverse();
      lcdClearScreen();
    break;
    
    case TOGGLE_SPLASH:
      toggleSplash();
    break;
    
    case ADJ_BL_LEVEL:
      while(1)  // Stay here until we are *told* to leave.
      {
        if (bufferSize > 0)
        {
          cmdBuffer[cmdBufferPtr++] = serialBufferPop();
        }
        if (cmdBufferPtr > 0)
        {
          cmdBufferPtr = 0;
          // We need to make sure our level never exceeds 100, or weird
          //   things can happen to the PWM generator.
          if ((uint8_t)cmdBuffer[0] > 100) cmdBuffer[0] = 100;
          BL_LEVEL = cmdBuffer[0];
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
            default:
            break;
          }
          break; // This is where we tell to code to leave the while loop.
        }
      }
    break;
    
    case ADJ_TEXT_X:
      while(1)  // Stay here until we are *told* to leave.
      {
        if (bufferSize > 0)
        {
          cmdBuffer[cmdBufferPtr++] = serialBufferPop();
        }
        if (cmdBufferPtr > 0)
        {
          cmdBufferPtr = 0;
          // The most recent byte is the new X origin of the text window- IF it
          //   makes sense. It only makes sense if it is at least 6 pixels from
          //   the right edge of the screen.
          if (cmdBuffer[0] <= 122)
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
        if (cmdBufferPtr > 0)
        {
          cmdBufferPtr = 0;
          // The most recent byte is the new Y origin of the text window- IF it
          //   makes sense. It only makes sense if it is at least 8 pixels from
          //   the bottom edge of the screen.
          if (cmdBuffer[1] <= 56)
          {
            textOrigin[1] = cmdBuffer[1];
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
        if (cmdBufferPtr > 2)
        {
          cmdBufferPtr = 0;
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
        if (cmdBufferPtr > 4)
        {
          cmdBufferPtr = 0;
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
        if (cmdBufferPtr > 3)
        {
          cmdBufferPtr = 0;
          if (cmdBuffer[4] == 0) pixel = OFF;
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
        if (cmdBufferPtr > 3)
        {
          cmdBufferPtr = 0;
          lcdEraseBlock(cmdBuffer[0], cmdBuffer[1], // start point x,y
                        cmdBuffer[2], cmdBuffer[3]); // end point x,y
          break; // This is where we tell to code to leave the while loop.
        }
      }
    break;
    
    default:
    break;
  }
}