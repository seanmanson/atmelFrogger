/*
 * buttons.c
 *
 * Author: Peter Sutton
 * Edited: Sean Manson
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "buttons.h"
#include "timer0.h"

// Global variable to keep track of the last button state so that we 
// can detect changes when an interrupt fires. The lower 4 bits (0 to 3)
// will correspond to the last state of port B pins 0 to 3.
static volatile uint8_t last_button_state;

// Our button queue. button_queue[0] is always the head of the queue. If we
// take something off the queue we just move everything else along. We don't
// use a circular buffer since it is usually expected that the queue is very
// short. In most uses it will never have more than 1 element at a time.
// This button queue can be changed by the interrupt handler below so we should
// turn off interrupts if we're changing the queue outside the handler.
#define BUTTON_QUEUE_SIZE 4
static volatile uint8_t button_queue[BUTTON_QUEUE_SIZE];
static volatile int8_t queue_length;

// Time (according to the clock) when the current button command should be repeated
// A value of 0 indicates that the buttons should not be repeated for now
static volatile uint32_t button_repeat_time;

// Button states for each button
// Each of these correspond to the expected input when only one button is pressed
#define BUTTON_B0 1
#define BUTTON_B1 2
#define BUTTON_B2 4
#define BUTTON_B3 8

// Delay values for repetition
// initial press
#define INIT_DELAY 300
// continual press
#define REPEAT_DELAY 150

// Setup interrupt if any of pins B0 to B3 change. We do this
// using a pin change interrupt. These pins correspond to pin
// change interrupts PCINT8 to PCINT11 which are covered by
// Pin change interrupt 1.
void init_button_interrupts(void) {
	// Disable interrupts so we can be sure that the interrupt
	// doesn't fire halfway through.
	uint8_t interruptsOn = bit_is_set(SREG, SREG_I);
	cli();
	
	// Enable the interrupt (see datasheet page 69)
	PCICR |= (1<<PCIE1);
	
	// Make sure the interrupt flag is cleared (by writing a 
	// 1 to it) (see datasheet page 69)
	PCIFR |= (1<<PCIF1);
	
	// Choose which pins we're interested in by setting
	// the relevant bits in the mask register (see datasheet page 70)
	PCMSK1 |= (1<<PCINT8)|(1<<PCINT9)|(1<<PCINT10)|(1<<PCINT11);	
	
	// Empty the button push queue
	queue_length = 0;
	
	// Clear the repeat time
	button_repeat_time = 0;
	
	// Reenable interrupts
	if(interruptsOn) {
		sei();
	}
}

int8_t button_pushed(void) {
	int8_t return_value = -1;	// Assume no button pushed
	if(queue_length > 0) {
		// Remove the first element off the queue and move all the other
		// entries closer to the front of the queue. We turn off interrupts (if on)
		// before we make any changes to the queue. If interrupts were on
		// we turn them back on when done.
		return_value = button_queue[0];
		
		// Save whether interrupts were enabled and turn them off
		int8_t interrupts_were_enabled = bit_is_set(SREG, SREG_I);
		cli();
		
		for(uint8_t i = 1; i < queue_length; i++) {
			button_queue[i-1] = button_queue[i];
		}
		queue_length--;
		
		if(interrupts_were_enabled) {
			// Turn them back on again
			sei();
		}
	}
	return return_value;
}

void activate_buttons(void) {
	// Disable interrupts so we can be sure that the interrupt
	// doesn't fire halfway through.
	uint8_t interruptsOn = bit_is_set(SREG, SREG_I);
	cli();
	
	// Enable the interrupt (see datasheet page 69)
	PCICR |= (1<<PCIE1);
	
	// Make sure the interrupt flag is cleared (by writing a
	// 1 to it) (see datasheet page 69)
	PCIFR |= (1<<PCIF1);
	
	// Empty the button push queue
	queue_length = 0;
	
	//Reenable interrupts
	if(interruptsOn) {
		sei();
	}
}

void deactivate_buttons(void) {
	// Disable interrupts so we can be sure that the interrupt
	// doesn't fire halfway through.
	uint8_t interruptsOn = bit_is_set(SREG, SREG_I);
	cli();
	
	// Disable the interrupt (see datasheet page 69)
	PCICR &= ~(1<<PCIE1);
	
	//Reenable interrupts
	if(interruptsOn) {
		sei();
	}
}

int8_t should_button_repeat(void) {
	// If we should not repeat this button
	if (button_repeat_time == 0) {
		return -1;
	}
	
	// Disable interrupts so we can be sure that the interrupt
	// doesn't fire halfway through.
	uint8_t interruptsOn = bit_is_set(SREG, SREG_I);
	cli();
	
	// Assume we should not repeat
	int8_t return_value = -1;
	
	// Get the current state of the buttons
	uint8_t button_state = PINB & 0x0F;
	
	// If there has been no changes in button state since last time, and
	// it is time to repeat a button movement
	if (get_clock_ticks() > button_repeat_time && button_state == last_button_state) {
		// Go through each of the 'only one button pushed' states
		// Return the value if it matches a single button state, and
		// set the timer for the next repeat.
		switch (button_state) {
			case BUTTON_B0:
				button_repeat_time = get_clock_ticks() + REPEAT_DELAY;
				return_value = 0;
				break;
			case BUTTON_B1:
				button_repeat_time = get_clock_ticks() + REPEAT_DELAY;
				return_value = 1;
				break;
			case BUTTON_B2:
				button_repeat_time = get_clock_ticks() + REPEAT_DELAY;
				return_value = 2;
				break;
			case BUTTON_B3:
				button_repeat_time = get_clock_ticks() + REPEAT_DELAY;
				return_value = 3;
				break;
			default:
				button_repeat_time = 0;
		}
	} else if (button_state != last_button_state) {
		// If the state has changed,
		// ignore the buttons and reset our repeat
		button_repeat_time = 0;
	}
	
	// Reenable interrupts
	if(interruptsOn) {
		sei();
	}
	return return_value;
}

// Interrupt handler for a change on buttons
ISR(PCINT1_vect) {
	// Get the current state of the buttons. We'll compare this with
	// the last state to see what has changed.
	uint8_t button_state = PINB & 0x0F;
	
	// If there is room in the queue:
	if(queue_length < BUTTON_QUEUE_SIZE) {
		// If the button state matches one of the specified single-button states,
		// add this button to the queue and start repeating
		switch (button_state) {
			case BUTTON_B0:
				button_queue[queue_length++] = 0;
				button_repeat_time = get_clock_ticks() + INIT_DELAY;
				break;
			case BUTTON_B1:
				button_queue[queue_length++] = 1;
				button_repeat_time = get_clock_ticks() + INIT_DELAY;
				break;
			case BUTTON_B2:
				button_queue[queue_length++] = 2;
				button_repeat_time = get_clock_ticks() + INIT_DELAY;
				break;
			case BUTTON_B3:
				button_queue[queue_length++] = 3;
				button_repeat_time = get_clock_ticks() + INIT_DELAY;
				break;
			default:
				button_repeat_time = 0;
		}
		
	}
	
	// Remember this button state
	last_button_state = button_state;
}