#include <AVR/interrupt.h>
extern volatile uint8_t 	rxRingBuffer[416];
extern volatile uint16_t 	rxRingHead;
extern volatile uint16_t	rxRingTail;
extern volatile uint16_t	bufferSize;

ISR(USART_RX_vect)
{
	if (rxRingHead++ == 416) rxRingHead = 0;
	bufferSize++;
	rxRingBuffer[rxRingHead] = UDR0;
}