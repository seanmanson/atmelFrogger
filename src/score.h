/*
 * score.h
 * 
 * Author: Sean Manson
 *
 * Contains basic score functionality.
 */

#ifndef SCORE_H_
#define SCORE_H_

#include <stdint.h>

#define BASE_SCORE_GET_TO_RIVERBANK 5
#define BASE_SCORE_COMPLETE_LEVEL 100

/* Defines the score, setting it to 0.
 */
void init_score(void);

/* Adds value to the score.
 */
void add_to_score(uint16_t value);

/* Returns the current score.
 */
uint16_t get_score(void);

#endif /* SCORE_H_ */