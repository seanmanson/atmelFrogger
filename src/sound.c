/*
 * sound.c
 *
 * Written by Sean Manson
 *
 * Buzzer controls.
 * 
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "sound.h"
#include "timer0.h"

// Our queue size is the maximum number of sounds which can be stored.
// Our buffer does not wrap around.
#define SOUND_QUEUE_SIZE 6

// We store the frequencies and times for each sound separately.
static uint16_t sound_frequencies_queue[SOUND_QUEUE_SIZE];
static uint8_t sound_times_queue[SOUND_QUEUE_SIZE];

// Current length of the queue.
static int8_t queue_length;

// Time to stop playing the current sound.
static int32_t stop_time;

// Minimum and maximum allowable frequencies + system clock
#define FREQ_MIN 150
#define FREQ_MAX 1000
#define SYS_CLK 8000000L

void init_buzzer(void) {
	// Disable interrupts so we can be sure that the interrupt
	// doesn't fire halfway through.
	uint8_t interruptsOn = bit_is_set(SREG, SREG_I);
	cli();
	
	// Clear the queue.
	queue_length = 0;
	
	// Clear the time to stop at.
	stop_time = 0;
	
	// Clear the timer.
	TCNT1 = 0;

	// Set the output compare value to match a default frequency.
	OCR1A = get_OCRB_value(FREQ_A4);
	
	// Set the timer to clear on compare match (CTC mode)
	// and to not divide the clock. This starts the timer
	// running.
	TCCR1A = 0;
	TCCR1B = (1 <<WGM12 | 1<<CS10);
	
	// Set D3 as an input.
	DDRD &= ~(1 << DDRD3);
	
	// Set D5, D7 as outputs.
	DDRD |= (1 << DDRD5 | 1 << DDRD7);
	PORTD &= ~(1 << DDRD7); //GND value for buzzer
	
	// Enable an interrupt on output compare match. 
	// Note that interrupts have to be enabled globally
	// before the interrupts will fire.
	TIMSK1 |= (1<<OCIE1A);
	
	// Make sure the interrupt flag is cleared by writing a 
	// 1 to it.
	TIFR1 &= (1<<OCF1A);
	
	//Reenable interrupts
	if(interruptsOn) {
		sei();
	}
}

void play_sound(uint16_t frequency, uint8_t time) {
	// Ignore invalid values
	if (frequency < FREQ_MIN || frequency > FREQ_MAX || queue_length >= SOUND_QUEUE_SIZE) {
		return;
	}
	
	// Add this sound to the queue.
	sound_frequencies_queue[queue_length] = frequency;
	sound_times_queue[queue_length] = time;
	queue_length++;
}

void play_quiet_sound(uint16_t frequency, uint8_t time) {
	// Ignore if other sounds are playing.
	if (queue_length > 0) {
		return;
	}
	
	// Add to queue.
	play_sound(frequency, time);
}

void clear_sounds() {
	queue_length = 0;
}

uint8_t is_playing_sound(void) {
	return (queue_length > 0);
}

/* HELPER FUNCTIONS */
uint8_t is_sound_on(void) {
	return (PIND & (1 << PIND3));
}

uint16_t get_OCRB_value(uint16_t frequency) {
	// Because we are toggling our sound, we want our interrupt to be
	// firing with a frequency twice that of the sound.
	// Also, OCR value = counts/interrupt = (counts/s)/(interrupts/s).
	// We then subtract 1 because we start counting from 0.
	return (SYS_CLK/(frequency*2)) - 1;
}

void start_next_sound(void) {
	// Clear the timer
	TCNT1 = 0;
	
	// Set the output compare value to match the next sound's frequency
	OCR1A = get_OCRB_value(sound_frequencies_queue[0]);
	
	// Start toggling the OCR1A pin (D5)
	start_toggling();
	
	// Set the time to stop playing
	stop_time = get_clock_ticks() + 10 * sound_times_queue[0];
	
	// Decrease queue length
	for(uint8_t i = 1; i < queue_length; i++) {
		sound_frequencies_queue[i-1] = sound_frequencies_queue[i];
		sound_times_queue[i-1] = sound_times_queue[i];
	}
	queue_length--;
}

void start_toggling(void) {
	TCCR1A |= (1<<COM1A0);
}

void stop_toggling(void) {
	TCCR1A &= ~(1<<COM1A0);
}

ISR(TIMER1_COMPA_vect) {
	// This interrupt should fire with the frequency of the current sound,
	// and serves to ensure that the current sound stops playing as neccessary.
	// It bases this off the values returned by timer 0.
	
	// Deactivate sound if D3 is not active.
	if (!is_sound_on()) {
		queue_length = 0;
		
		// Stop toggling the OCR1A pin (D5)
		stop_toggling();
		return;
	}
	
	// If we are now past the time for this sound to stop:
	if (get_clock_ticks() > stop_time) {
		// If this isn't the last sound, go to the next.
		// Otherwise, stop playing sounds.
		if (queue_length != 0) {
			start_next_sound();
		} else {
			// Stop toggling the OCR1A pin (D5)
			stop_toggling();
		}
	}
}


/* In-built tunes */
void play_tune_startup(void) {
	clear_sounds();
	play_sound(FREQ_F5, 37);
	play_sound(FREQ_C5SHARP, 10);
	play_sound(FREQ_D5SHARP, 30);
	play_sound(FREQ_G5SHARP, 25);
	play_sound(FREQ_F5, 25);
	play_sound(FREQ_C5SHARP, 40);
}
void play_tune_success(void) {
	clear_sounds();
	play_sound(FREQ_C5SHARP, 10);
	play_sound(FREQ_D5SHARP, 30);
	play_sound(FREQ_G5SHARP, 25);
	play_sound(FREQ_B5, 40);
}
void play_tune_dead(void) {
	clear_sounds();
	play_sound(FREQ_F5, 15);
	play_sound(FREQ_G5SHARP, 10);
	play_sound(FREQ_C5SHARP, 10);
	play_sound(FREQ_B4, 40);
}
void play_tune_lost(void) {
	clear_sounds();
	play_sound(FREQ_G5SHARP, 10);
	play_sound(FREQ_C5SHARP, 10);
	play_sound(FREQ_B4, 10);
	play_sound(FREQ_G4, 40);
}