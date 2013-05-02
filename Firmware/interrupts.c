#include <AVR/interrupt.h>
#include "serial.h"
#include "glcdbp.h"

extern volatile uint8_t 	rxRingBuffer[BUF_DEPTH];
extern volatile uint16_t 	rxRingHead;
extern volatile uint16_t	rxRingTail;
extern volatile uint16_t	bufferSize;

// Handler for USART receive interrupts. This is basically just a stack push
//  for the FIFO we use to store incoming commands. Note that there is no
//  overflow; that might be a nice touch but so far, I haven't even come close
//  to hitting the buffer depth. In fact, I've never exceeded a depth of more
//  than two or three bytes.
ISR(USART_RX_vect)
{
	if (rxRingHead == BUF_DEPTH) rxRingHead = 0;
	bufferSize++;
	rxRingBuffer[rxRingHead++] = UDR0;
}
