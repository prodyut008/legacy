#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <string.h>

#include "radiocontrol.h"

void radio_init ( void )
{
	cli();
	TCCR0B |= (1<<CS01);// | (1<<CS00);  // clk / 64
	TCCR0A |= (1<<WGM01); // ctc mode
	TIMSK  |= (1<<OCIE0A);
	TIFR   |= (1<<OCF0A);
	OCR0A = RADIO_SEND_TIME;
	sei();
}

const uint8_t sendcode[5] = { 0x65, 0xb2, 0xcb, 0x2c, 0xb0 };

ISR(TIMER0_COMPA_vect)
{
	static uint8_t bytecounter = 0, bitmask = 0x80;

	if (bytecounter == 0)
	{
		if (bitmask == 0x01)
			PORTD |= _BV(PD6);

	} else if (bytecounter < 6)
	{
		if (sendcode[bytecounter-1] & bitmask)
		{
			PORTD |= _BV(PD6);
		} else
		{
			PORTD &= ~(_BV(PD6));
		}
	} else
	{
		PORTD &= ~(_BV(PD6));
		if (bytecounter == 10)
		{
			bytecounter = 0;
			bitmask = 0x80;
		}
	}
	
	bitmask = bitmask >> 1;

	if (bitmask == 0x00)
	{
		bitmask = 0x80;
		bytecounter++;
	}
}
