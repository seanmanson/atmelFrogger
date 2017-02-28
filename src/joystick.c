/*
 * joystick.c
 *
 * Author: Sean Manson
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "joystick.h"
#include "timer0.h"

static volatile uint16_t last_x; // The last x value of the joystick
static volatile uint16_t last_y; // The last y value of the joystick
static volatile uint8_t x_or_y; // Whether or not the last value calculated was for x or y
static uint8_t last_joystick_zone; // The last zone the joystick was in
static uint32_t last_joystick_time; // The last time (clock tick) the joystick incurred movement
									// A value of 0 implies the joystick should not be repeating
static uint8_t movement_value; // The last direction the joystick wanted to move in

// The delay between joystick movement repetitions
#define REPEAT_DELAY 250

void init_joystick(void) {
	// Disable interrupts so we can be sure that the interrupt
	// doesn't fire halfway through.
	uint8_t interruptsOn = bit_is_set(SREG, SREG_I);
	cli();
	
	// Set up ADC - AVCC reference, right adjust
	// We set our input selection for X on A6
	ADMUX = (1<<REFS0)|(1<<MUX2)|(1<<MUX1);
	
	// Turn on the ADC (but don't start a conversion yet).
	// Set up the conversion complete interrupt.
	// Choose a clock divider of 64.
	ADCSRA = (1<<ADEN)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1);
	
	// Set starting values for this joystick
	last_x = 512;
	last_y = 512;
	x_or_y = 0; //x
	last_joystick_zone = CENTRE;
	last_joystick_time = 0;
	movement_value = CENTRE;
	
	// Reenable interrupts
	if(interruptsOn) {
		sei();
	}
	
	// Start the ADC conversion
	ADCSRA |= (1<<ADSC);
}

uint8_t should_joystick_move(void) {
	// Disable interrupts so we can be sure that the interrupt
	// doesn't fire halfway through.
	uint8_t interruptsOn = bit_is_set(SREG, SREG_I);
	cli();
	
	// By default we want to remain where we are
	movement_value = CENTRE;
	
	// Get what zone the joystick is in
	uint8_t current_zone = get_current_zone();
	
	if ((last_joystick_time != 0) && (get_clock_ticks() > last_joystick_time + REPEAT_DELAY)) {
		// If we are currently repeating joystick movement, and
		// it is past the time to repeat movement:
		if (current_zone == CENTRE) {
			// If we are currently in the centre,
			// do nothing, and reset our joystick repeat.
			last_joystick_time = 0;
		} else {
			// Otherwise,
			// move, and get ready to repeat next time.
			movement_value = current_zone;
			last_joystick_time = get_clock_ticks();
		}
	} else if (last_joystick_time == 0) {
		// If we are not currently repeating:
		if (last_joystick_zone == CENTRE && current_zone != CENTRE) {
			// If we are moving out of the centre,
			// move in our desired direction and start the repeat timer
			movement_value = current_zone;
			last_joystick_time = get_clock_ticks();
		}
	}
	
	// Update our previous zone value
	last_joystick_zone = current_zone;
	
	// Reenable interrupts
	if(interruptsOn) {
		sei();
	}
	return (movement_value != CENTRE); // True if we are not staying where we are
}

uint8_t get_last_joystick_movement_value(void) {
	// Return the value set by the above function
	return movement_value;
}

uint16_t get_last_x(void) {
	return last_x;
}

uint16_t get_last_y(void) {
	return last_y;
}


/* HELPER FUNCTIONS */
uint8_t get_current_zone(void) {
	// Go systematically through each zone, starting from the corners
	if (last_x < (CENTRE_MID_X - DEAD_ZONE_DIAG) && last_y < (CENTRE_MID_Y - DEAD_ZONE_DIAG)) {
		return TOPLEFT;
	}
	if (last_x > (CENTRE_MID_X + DEAD_ZONE_DIAG) && last_y < (CENTRE_MID_Y - DEAD_ZONE_DIAG)) {
		return TOPRIGHT;
	}
	if (last_x < (CENTRE_MID_X - DEAD_ZONE_DIAG) && last_y > (CENTRE_MID_Y + DEAD_ZONE_DIAG)) {
		return BOTTOMLEFT;
	}
	if (last_x > (CENTRE_MID_X + DEAD_ZONE_DIAG) && last_y > (CENTRE_MID_Y + DEAD_ZONE_DIAG)) {
		return BOTTOMRIGHT;
	}
	if (last_y < (CENTRE_MID_Y - DEAD_ZONE_Y)) {
		return TOP;
	}
	if (last_y > (CENTRE_MID_Y + DEAD_ZONE_Y)) {
		return BOTTOM;
	}
	if (last_x < (CENTRE_MID_X - DEAD_ZONE_X)) {
		return LEFT;
	}
	if (last_x > (CENTRE_MID_X + DEAD_ZONE_X)) {
		return RIGHT;
	}
	return CENTRE;
}

// Interrupt handler for a conversion complete
ISR(ADC_vect) {
	// Take the value out of the converter
	uint16_t value = ADC;
	
	// Update x/y values
	if (x_or_y == 0) {
		last_x = value;
	} else {
		last_y = HEIGHT - value; // Flip this to go up->down rather than down->up
	}
	
	// Change conversion to other pin
	x_or_y = 1 - x_or_y;
	if (x_or_y == 0) { //y->x
		ADMUX &= ~1;
	} else { //x->y
		ADMUX |= 1;
	}
		
	//Start the ADC conversion once more
	ADCSRA |= (1<<ADSC);
}