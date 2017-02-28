/*
 * level.h
 * 
 * Author: Sean Manson
 *
 * Tackles with the level system, containing methods for getting and setting levels.
 * Also contains the acceleration of the game, which is stored as 'difficulty'.
 * 'Difficulty' is current speed of the game. It is stored as 100*the speed factor.
 * Every level this difficulty increases by a ramp-up factor, which decreases as the
 * game gets harder before the difficulty caps out at 500.
 * 
 */

#ifndef LEVEL_H_
#define LEVEL_H_

#include <stdint.h>

/* Define the internal level system, setting default values.
 */
void init_level(void);

/* Go up a level, increasing difficulty.
 * Does nothing if max level is reached.
 */
void increment_level(void);

/* Returns the current level number.
 */
uint8_t get_level(void);

/* Flips the direction of lane movement in this level.
 */
void flip_level_direction(void);

/* Returns the direction of lane movement in this level.
 * 1 = standard, -1 = reversed
 */
int8_t get_level_direction(void);

/* Returns the current difficulty, as a value ready to be divided by 1000.
 */
uint16_t get_difficulty(void);

/* Returns the ones place of the speed factor.
 * The speed factor is the current speed of the game, and is equal to
 * 100 / difficulty.
 */
uint8_t get_factor_ones(void);

/* Returns the tenths and hundredths place of the speed factor.
 * The speed factor is the current speed of the game, and is equal to
 * 100 / difficulty.
 *
 * Note that this does not come with leading zeros--these must be added later.
 */
uint8_t get_factor_tenthshundreths(void);

/* Helper functions */
uint16_t get_current_ramp_up(void);

#endif /* LEVEL_H_ */