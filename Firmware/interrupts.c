#include <AVR/interrupt.h>
#include "serial.h"
#include "glcdbp.h"

extern volatile uint8_t 	rxRingBuffer[BUF_DEPTH];
extern volatile uint16_t 	rxRingHead;
extern volatile uint16_t	rxRingTail;
extern volatile uint16_t	bufferSize;

ISR(USART_RX_vect)
{
	if (rxRingHead == BUF_DEPTH) rxRingHead = 0;
	bufferSize++;
	rxRingBuffer[rxRingHead++] = UDR0;
}
