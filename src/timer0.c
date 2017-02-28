/*
 * timer0.c
 *
 * Author: Peter Sutton
 * Edited by Sean Manson
 *
 * We setup timer0 to generate an interrupt every 1ms
 * We update a global clock tick variable - whose value
 * can be retrieved using the get_clock_ticks() function.
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "timer0.h"

/* Our internal clock tick count - incremented every 
 * millisecond. Will overflow every ~49 days. */
static volatile uint32_t clockTicks;

/* Our in-game counter, which is similar to the above but only
 * runs while our game is active. */
static volatile uint32_t inGameClockTicks;
static uint8_t ingame_timer_is_counting = 0; //is this timer active?

// Countdown timer, in milliseconds.
static volatile uint16_t countdown = 0;

// Current digit being displayed for the countdown
// 0, 1 for ones; 2, 3 for tens
// We work with two values to halve the switching speed and remove ghosting
static volatile uint8_t digit_counter = 0;

/* Seven segment display segment values for 0 to 9 */
static const uint8_t seven_seg_data[10] = {63,6,91,79,102,109,125,7,127,111};


/* Set up timer 0 to generate an interrupt every 1ms. 
 * We will divide the clock by 64 and count up to 124.
 * We will therefore get an interrupt every 64 x 125
 * clock cycles, i.e. every 1 milliseconds with an 8MHz
 * clock. 
 * The counter will be reset to 0 when it reaches it's
 * output compare value.
 */
void init_timer0(void) {
	/* Reset clock tick count. L indicates a long (32 bit) 
	 * constant. 
	 */
	clockTicks = 0L;
	inGameClockTicks = 0L;
	
	/* Clear the timer */
	TCNT0 = 0;

	/* Set the output compare value to be 124 */
	OCR0A = 124;
	
	/* Set the timer to clear on compare match (CTC mode)
	 * and to divide the clock by 64. This starts the timer
	 * running.
	 */
	TCCR0A = (1<<WGM01);
	TCCR0B = (1<<CS01)|(1<<CS00);

	/* Enable an interrupt on output compare match. 
	 * Note that interrupts have to be enabled globally
	 * before the interrupts will fire.
	 */
	TIMSK0 |= (1<<OCIE0A);
	
	/* Make sure the interrupt flag is cleared by writing a 
	 * 1 to it.
	 */
	TIFR0 &= (1<<OCF0A);
}

uint32_t get_clock_ticks(void) {
	uint32_t returnValue;

	/* Disable interrupts so we can be sure that the interrupt
	 * doesn't fire when we've copied just a couple of bytes
	 * of the value. Interrupts are re-enabled if they were
	 * enabled at the start.
	 */
	uint8_t interruptsOn = bit_is_set(SREG, SREG_I);
	cli();
	returnValue = clockTicks;
	if(interruptsOn) {
		sei();
	}
	return returnValue;
}

uint32_t get_ingame_clock_ticks(void) {
	uint32_t returnValue;

	/* Disable interrupts so we can be sure that the interrupt
	 * doesn't fire when we've copied just a couple of bytes
	 * of the value. Interrupts are re-enabled if they were
	 * enabled at the start.
	 */
	uint8_t interruptsOn = bit_is_set(SREG, SREG_I);
	cli();
	returnValue = inGameClockTicks;
	if(interruptsOn) {
		sei();
	}
	return returnValue;
}

void start_ingame_timer(void) {
	ingame_timer_is_counting = 1;
}

void stop_ingame_timer(void) {
	ingame_timer_is_counting = 0;
}


/* COUNTDOWN FUNCTIONS */
void init_countdown(void) {
	/* Make all bits of port C be output bits */
	DDRC = 0xFF;
	//Make CC an output bit
	DDRD |= (1 << DDRD2);
	countdown = 0;
}

void countdown_set(uint8_t start) {
	countdown = start*1000;
}

void countdown_clear(void) {
	countdown = 0;
}

uint8_t is_countdown_done(void) {
	return (countdown == 0);
}

uint8_t get_countdown_time_remaining(void) {
	return countdown/1000;
}


ISR(TIMER0_COMPA_vect) {
	/* Increment our clock tick count */
	clockTicks++;
	
	if (ingame_timer_is_counting) {
		inGameClockTicks++;
		if (countdown > 0) {
			//decrement the countdown
			countdown--;
		}
	}
	
	/* Change which digit will be displayed:
	 * 0, 1 = right digit
	 * 2, 3 = left digit
	 */
	digit_counter++;
	if (digit_counter > 3) {
		digit_counter = 0;
	}
	uint8_t seven_seg_cc = digit_counter >> 1;
	
	if(countdown > 0) { //if we are counting down
		uint16_t to_display = countdown + 1000;
		//led display
		if(seven_seg_cc == 0) {
			/* Display rightmost digit - seconds */
			// We add one to remove the rounding down side effect
			// Without it, '00' displays for a whole second before stopping
			PORTC = seven_seg_data[(to_display/1000)%10];
			
		} else {
			/* Display leftmost digit - tens of seconds */
			if (to_display > 10000) { //only display if it isn't a 0
				PORTC = seven_seg_data[(to_display/10000)%10];
			} else {
				PORTC = 0;
			}
			
		}
	} else {
		PORTC = 0;
	}
	
	/* Output the digit selection (CC) bit */
	if (seven_seg_cc) {
		PORTD |= (1 << PORTD2);
	} else {
		PORTD &= ~(1 << PORTD2);
	}
}
