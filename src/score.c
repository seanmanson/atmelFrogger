/*
 * score.c
 *
 * Written by Sean Manson
 */

#include "score.h"

uint16_t score;

void init_score(void) {
	score = 0;
}

void add_to_score(uint16_t value) {
	score += value;
}

uint16_t get_score(void) {
	return score;
}
