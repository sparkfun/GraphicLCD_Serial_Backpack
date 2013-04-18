#include "ui.h"
#include "lcd.h"
#include "serial.h"
#include "glcdbp.h"

// These variables are defined in glcdbp.c, and are used for the input buffer
//   from the serial port. We need to be able to access them here because we'll
//   be stuck here parsing input from the serial port once a command comes in.
extern volatile uint8_t 	rxRingBuffer[416];
extern volatile uint16_t 	rxRingHead;
extern volatile uint16_t	rxRingTail;
extern volatile uint16_t	bufferSize;

void uiStateMachine(char command)
{
  char cmdBuffer[5];
  uint8_t cmdBufferPtr = 0;
  switch(command)
  {
    case CLEAR_SCREEN:
    lcdClearScreen();
    break;
    
    case RUN_DEMO:
    // to be implemented
    break; 
    
    case TOGGLE_BGND:
    // to be implemented
    break;    
    
    case TOGGLE_SPLASH:
    // to be implemented
    break;  
    
    case ADJ_BL_LEVEL:
    // to be implemented
    break;   
    
    case ADJ_BAUD_RATE:
    // to be implemented
    break;  
    
    case ADJ_TEXT_X:
    
    break;     
    
    case ADJ_TEXT_Y:
    
    break;     
    
    case DRAW_PIXEL:
      putChar('p');
      while(bufferSize == 0);
      while((bufferSize > 0)||(cmdBufferPtr < 3))
      {
        cmdBuffer[cmdBufferPtr++] = serialBufferPop();
        if (cmdBufferPtr > 3)
        {
          cmdBufferPtr = 0;
          lcdDrawPixel(cmdBuffer[0], cmdBuffer[1], (PIX_VAL)cmdBuffer[2]);
          break;
        }
      }
    break;  
    
    case DRAW_LINE:
    
    break;   
    
    case DRAW_CIRCLE:
    
    break;    
    
    case DRAW_BOX:
    
    break;      
    
    case ERASE_BLOCK:
    
    break;
    default:
    break;
  }
}