/***************************************************************************
glcdbp.c

Main application file for the serial graphical LCD backpack project. This is
 the main application loop, as well as the calling function for all init
 code found elsewhere. The PWM timer init code is here, too, because I wasn't
 sure where else to put.

02 May 2013 - Mike Hord, SparkFun Electronics

This code is released under the Creative Commons Attribution Share-Alike 3.0
 license. You are free to reuse, remix, or redistribute it as you see fit,
 so long as you provide attribution to SparkFun Electronics.

***************************************************************************/

#include <avr/interrupt.h>
#include <util/delay.h>
#include "glcdbp.h"
#include "io_support.h"
#include "serial.h"
#include "lcd.h"
#include "ui.h"
#include "nvm.h"

// These variables will be used over and over, in various files, to access
//  global variables that may be needed to make decisions elsewhere.
enum DISPLAY_TYPE   display = SMALL;
volatile uint8_t    rxRingBuffer[BUF_DEPTH];
volatile uint8_t    bufferSize = 0;
volatile uint16_t   rxRingHead = 0;
volatile uint16_t   rxRingTail = 0;
volatile uint8_t    reverse = 0;

int main(void)
{
  // The first thing we want to check is if we have a large or small
  //  display on our hands. We can tell because PB3 will be pulled high if
  //  the display is large (hopefully; that's done at build time).
  PORTB |= 0x08;   // Enable the pull-up on PB3.
  _delay_us(5);    // Wait a few us for the pin to change- this is important!
  uint8_t portTemp = PINB;  // Cache the pins status...
  if ((portTemp & 0x08) == 0x08)  // ...and check to see if what the pin says.
  {
    display = LARGE;  // If the pin is high, call it a large display.
  }
  else 
  {
    display = SMALL;  // If the pin is low, it's a small display.
  }
  PORTB &= ~0x08; // Disable the pull-up on PB3.
  
  // ioInit() configures the IO pins as we'll need them for the rest of the
  //  code; once we've identified our display size, we'll do the pins
  //  accordingly.
  ioInit();
  
  // We use a timer (timer1) for our PWM of the backlight. This function sets
  //  that up as needed.
  timerInit();
  
  // By default, we want to start up at 115200BPS. In a second, we'll check
  //  for any input from the user to see if we should remain at 115200 or
  //  check the EEPROM for a different speed.
  serialInit(BR115200);
  
  // Enable interrupts. The only thing we use interrupts for is serial data.
  sei();
  
  // Check to see if we want to be in white-on-black mode (1) or black-on-white
  //   mode (reverse = 0).
  reverse = 0x01 & getReverse();
  
  // Configure the LCD as it should be configured; we leave this until late
  //  in the process more as a relic of development than anything else, b/c
  //  it was useful to have the other stuff setup first.
  lcdConfig();
  
  // Clear the screen for good measure. This *may* not be necessary, but for
  //  the large display, it definitely is, as the power-on status of that one
  //  is pure garbage, everywhere.
  lcdClearScreen();
  
  // Draw the splash, if the EEPROM value says we should.
  if ((getSplash() & 0x01)==1) lcdDrawLogo();
  
  // Now wait for one second, for the user to override the stored baud rate
  //  and get back to 115200, if they so desire.
  _delay_ms(1000);
  
  // If the user has send *any* character during the splash time, we should
  //  skip this switch and set our baud rate back to 115200.
  if (bufferSize == 0)
  {
    switch(getBaudRate())
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
  }
  else setBaudRate('6');
  
  // Clear off the splash.
  lcdClearScreen();
  
  // Clear the serial buffer. Any data sent during that initial 1s period is
  //  lost; we don't care because we know that data *should* only be for an
  //  emergency baud rate reset.
  clearBuffer();
  
  // This is a troubleshooting message, to let us know that setup has
  //  completed and the display is ready to rock. It is commented in the
  //  production version of the code but left in place.
  // putLine("Ready to serve!");
  
  // Under normal circumstances, we'll spend *all* our time cycling through
  //  this loop, parsing input from the serial port. The serial data is
  //  buffered by an interrupt, and we'll pop that buffer here (and in ui.c)
  //  and make decisions on what to do with it.
  while(1)
  {
    // If there's *anything* in the buffer, we need to deal with it.
    while (bufferSize > 0)
    {
      // serialBufferPop() pulls data from the top of the FIFO that comprises
      //  our serial port buffer, automatically changing the pointers and
      //  stack size.
      char bufferChar = serialBufferPop();
      // If the character received is the command escape character ('|')...
      if (bufferChar == '|')
      {
        while (bufferSize == 0);    // ...wait for the next character...
        bufferChar = serialBufferPop(); // ...fetch the character..
        uiStateMachine(bufferChar); // ... then see what to do.
        // Note that we won't return from the state machine until the command
        //  specified by the character that sends us there has been completed-
        //  there's no bailing out of that process. Yet.
      }
      // Otherwise, draw the character. lcdDrawChar also handles backspace,
      //   carriage return and new line.
      else if (((bufferChar >= ' ') && (bufferChar <= '~')) ||
               (bufferChar == '\r') ||  // Newline.
               (bufferChar == '\b') )   // Backspace.
        lcdDrawChar(bufferChar);
    }
  }
}

void timerInit(void)
{  
  // Timer1 initialization
  //  We use timer 1 fast PWM mode to dim the backlight on the display.
  //  OC1B (PB2) is connected to a BJT for controlling the backlight; the BJT
  //  is PNP so we want to use inverting mode.
  // PWM frequency is fclk/(N*(1+TOP)), where TOP is, in this case 100,
  //  N = 1, and fclk is 16MHz. Thus, Fpwm ~ 160kHz.
  
  // TCCR1A-  7:6 - Channel A compare output mode
  //                 Set to 00 for normal pin operation
  //          5:4 - Channel B compare output mode
  //                 Set to 01 for inverting PWM output mode
  //          3:2 - Don't care/no use
  //          1:0 - Waveform generation mode bits 1:0
  //                 Along with WGM1 3:2 (In TCCR1B), set to 1111 to enable
  //                 fast PWM mode. TCNT1 will increment until it reaches ICR1,
  //                 then reset, and the pin will change when TCNT1 == 0 and
  //                 when TCNT1 == OCR1B.
  TCCR1A = 0b00110010;
  
  // TCCR1B-  7   - Input noise canceler (Don't care)
  //          6   - Input capture edge select (Don't care)
  //          5   - Don't care/no use
  //          4:3 - Waveform generation mode bits 3:2
  //                 See above; set to 11 for fast PWM
  //          2:0 - Timer 1 clock source
  //                 Set to 001 for no clock divisor.
  TCCR1B = 0b00011001;
  
  // ICR1-  Really implemented as two 8-bit registers (ICR1L and ICR1H),
  //  the value in this register (in this mode) marks the point at which
  //  the timer quits counting and returns to zero. By making it 100, we
  //  can then really easily set our backlight intensity from 0-100.
  ICR1 = 100;
  
  // OCR1B- Really implemented as two 8-bit registers (OCR1BL and OCR1BH),
  //  the value in this register is the point where the output pin will
  //  transition from low to high, turning the backlight off. We have a
  //  value stored in EEPROM, so we need to retrieve it.
  OCR1B = getBacklightLevel();
}


