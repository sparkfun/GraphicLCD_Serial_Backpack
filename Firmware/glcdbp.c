#include <avr/pgmspace.h> 
#include <avr/io.h>
#include <math.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include "serial.h"
#include "glcdbp.h"
#include "t6963.h"

uint8_t BL_dutycycle = 100;

enum DISPLAY_TYPE display = SMALL;
volatile uint8_t 	rxRingBuffer[416];
volatile uint16_t 	rxRingHead = 0;
volatile uint16_t	rxRingTail = 0;
volatile uint16_t	bufferSize = 0;

int main(void)
{
	serialInit(BR115200);
	ioInit();
	timerInit();
	putChar('!');
	while(1)
	{
	}
}

void ioInit(void)
{
	// Set up the data direction registers for the data bus pins.
	//  The data bus is on PB0:1 and PD2:7, so make those pins outputs.
	DDRB = 0b00000011;
	DDRD = 0b11111100;
	
	DDRB |= (1<<nBL_EN);		// Set PB2 (backlight enable) as an output
	//PORTB |= (1<<nBL_EN);		// Turn backlight off
	
	// Now we need to configure the I/O to support the two types of display.
	if (display == SMALL)
	{
		DDRC =  ((1<<EN) | (1<<RS) | (1<<R_W) | (1<<RESET) | (1<<CS1) | (1<<CS2));
		PORTC = ((1<<EN) | (1<<RS) | (1<<R_W) | (1<<RESET) | (1<<CS1) | (1<<CS2));
	}
	else if (display == LARGE)
	{
		DDRC =  ((1<<WR) | (1<<RD) | (1<<CE) | (1<<CD) | (1<<HALT) | (1<<RST));
		PORTC = ((1<<WR) | (1<<RD) | (1<<CE) | (1<<CD) | (1<<HALT) | (1<<RST));
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
	OCR1B = 25;//eeprom_read_word(BACKLIGHT_VALUE);
	
}
