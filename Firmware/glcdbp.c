#include <avr/interrupt.h>
#include <util/delay.h>
#include "glcdbp.h"
#include "io_support.h"
#include "serial.h"
#include "lcd.h"
#include "ui.h"
#include "nvm.h"

enum DISPLAY_TYPE   display = SMALL;
volatile uint8_t 	  rxRingBuffer[416];
volatile uint16_t	  bufferSize = 0;
uint16_t            rxRingHead = 0;
uint16_t            rxRingTail = 0;

int main(void)
{
	ioInit();
	lcdConfig();
	timerInit();
	serialInit(BR115200);
	sei();
  _delay_ms(1000);
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
  
  clearBuffer();
  
  putLine((char*)"Ready to serve!");
	while(1)
	{
		while (bufferSize > 0)
		{
			char bufferChar = serialBufferPop();
      if (bufferChar < ' ')
        uiStateMachine(bufferChar);
      else if ((bufferChar >= ' ') && (bufferChar <= '~'))
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
	//	N = 1, and fclk is 16MHz. Thus, Fpwm ~ 160kHz.
	
	// TCCR1A-  7:6 - Channel A compare output mode
	//				Set to 00 for normal pin operation
	//		    5:4 - Channel B compare output mode
	//				Set to 01 for inverting PWM output mode
	//		    3:2 - Don't care/no use
	//			1:0 - Waveform generation mode bits 1:0
	//				Along with WGM1 3:2 (In TCCR1B), set to 1111 to enable fast
	//				PWM mode. TCNT1 will increment until it reaches ICR1, then
	//              reset, and the pin will change when TCNT1 == 0 and when
	//				TCNT1 == OCR1B.
	TCCR1A = 0b00110010;
	
	// TCCR1B-	7   - Input noise canceler (Don't care)
	//			6	- Input capture edge select (Don't care)
	//			5	- Don't care/no use
	// 			4:3 - Waveform generation mode bits 3:2
	//				See above; set to 11 for fast PWM
	//			2:0 - Timer 1 clock source
	//				Set to 001 for no clock divisor.
	TCCR1B = 0b00011001;
	
	// ICR1-	Really implemented as two 8-bit registers (ICR1L and ICR1H),
	//	the value in this register (in this mode) marks the point at which
	//  the timer quits counting and returns to zero. By making it 100, we
	//  can then really easily set our backlight intensity from 0-100.
	ICR1 = 100;
	
	// OCR1B- Really implemented as two 8-bit registers (OCR1BL and OCR1BH),
	//	the value in this register is the point where the output pin will
	//  transition from low to high, turning the backlight off. We have a
	//  value stored in EEPROM, so we need to retrieve it.
	OCR1B = getBacklightLevel();
}


