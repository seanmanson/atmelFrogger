/*
 * lives.h
 * 
 * Author: Sean Manson
 *
 * Contrary to most game systems, internally the player starts with _4_
 * lives and they lose once this value reaches 0. When this value is displayed
 * on the LEDs, it is shown as one less than the internal value. By
 * implementing it this way, we remove the need to deal with negative
 * values.
 *
 * Lives are lost when the player dies. When this occurs, a message is
 * first given to the player telling them that they have lost, and then
 * after receiving button confirmation telling the game to continue,
 * the game resumes where it was with a new frog and one less life.
 *
 * This file also sets up the IO for displaying to the LEDs, as well as
 * containing the code for displaying them. These LEDs are connected to 
 * outputs A0-A3 on the board.
 */

#ifndef LIVES_H_
#define LIVES_H_

#include <stdint.h>
#include <avr/io.h>

void init_lives_display(void);

void init_lives(void);
void lose_life(void);
void gain_life(void);
uint8_t get_lives(void);
uint8_t get_at_max_lives(void);
uint8_t player_has_lost(void);

void update_lives_display(void);

#endif /* LIVES_H_ */