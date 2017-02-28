/*
 * level.c
 *
 * Written by Sean Manson
 */

#include "level.h"

// Level to start on
#define STARTING_LEVEL 1
// Maximum level reachable
#define MAX_LEVEL 99
// Difficulty to start on
#define STARTING_DIFFICULTY 100
// Percent by which this difficulty is increased as time goes on
#define RAMP_UP_FACTOR_100 25
#define RAMP_UP_FACTOR_200 10
#define RAMP_UP_FACTOR_300 5
#define RAMP_UP_FACTOR_400 2
#define RAMP_UP_FACTOR_500 0

// Directions
#define DIRECTION_STANDARD 1
#define DIRECTION_REVERSE -1

uint8_t level; // Current level
uint16_t difficulty; // Current difficulty
int8_t direction; // Direction lanes on this level should be going.


// Set the level to the starting level
void init_level(void) {
	level = STARTING_LEVEL;
	difficulty = STARTING_DIFFICULTY;
	direction = DIRECTION_STANDARD;
}

// Increment the current level by 1
void increment_level(void) {
	if (level < MAX_LEVEL) {
		level++;
		difficulty += get_current_ramp_up();
	}
}

// Return the current level
uint8_t get_level(void) {
	return level;
}

void flip_level_direction(void) {
	direction = -direction;
}

int8_t get_level_direction(void) {
	return direction;
}

// Return the current difficulty
uint16_t get_difficulty(void) {
	return difficulty;
}

// Get the factor values for displaying the current speed
uint8_t get_factor_ones(void) {
	return difficulty/100;
}
uint8_t get_factor_tenthshundreths(void) {
	return difficulty%100;
}

/* Helper functions */
uint16_t get_current_ramp_up(void) {
	if (difficulty >= 500) {
		return RAMP_UP_FACTOR_500;
	} else if (difficulty >= 400) {
		return RAMP_UP_FACTOR_400;
	} else if (difficulty >= 300) {
		return RAMP_UP_FACTOR_300;
	} else if (difficulty >= 200) {
		return RAMP_UP_FACTOR_200;
	} else {
		return RAMP_UP_FACTOR_100;
	}
}