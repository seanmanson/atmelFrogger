/*
 * lives.c
 *
 * Written by Sean Manson
 */

#include "lives.h"

#define BASE_STARTING_LIVES 4
#define MAX_LIVES 5

uint8_t lives = 0; //display nothing by default

// Set up the registers to display lives to LEDs
void init_lives_display(void) {
	//A0 - A3 are outputs
	DDRA |= 0x0F;
	PORTA &= 0xF0;
}

// Reset lives to the starting value
void init_lives(void) {
	lives = BASE_STARTING_LIVES;
	update_lives_display();
}

// Detract a life and update display
void lose_life(void) {
	if (lives > 0) {
		lives--;
	}
	update_lives_display();
}

// Add a life and update display
void gain_life(void) {
	if (lives < MAX_LIVES) {
		lives++;
	}
	update_lives_display();
}

// Get the number of lifes
// Given as the internal number of lives - 1
uint8_t get_lives(void) {
	return (lives - 1);
}

// Returns whether we are at our maximum lives.
uint8_t get_at_max_lives(void) {
	return (lives == MAX_LIVES);
}

// Returns true if the player is out of lives
uint8_t player_has_lost(void) {
	return (lives == 0);
}

/* PRIVATE FUNCTIONS */
// Displays the current number of lives on the LEDs
// Works with one less than the internal number
void update_lives_display(void) {
	PORTA &= 0xF0; //clear lives
	uint8_t i;
	for(i=0;i<(lives-1);i++) {
		PORTA |= (1 << i);
	}
}
