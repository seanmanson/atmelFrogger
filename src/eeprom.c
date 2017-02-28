/*
 * eeprom.c
 *
 * Author: Sean Manson
 */ 

#include "eeprom.h"

#include <string.h>
#include <stdio.h>

#include <avr/eeprom.h>

#define EEPROM_SIGNATURE 8
#define EEPROM_SIGNATURE_LENGTH 8
#define EEPROM_NAMES 16
#define EEPROM_NAMES_LENGTH 105
#define EEPROM_SCORES 121
#define EEPROM_SCORES_LENGTH 10
#define EEPROM_LEVELS 131
#define EEPROM_LEVELS_LENGTH 5

static char highscore_names[HIGHSCORES_TO_STORE][HIGHSCORE_NAME_LENGTH+1];
static uint16_t highscore_scores[HIGHSCORES_TO_STORE];
static uint8_t highscore_levels[HIGHSCORES_TO_STORE];
static const char signature[8] = "Twigged";

void init_highscores(void) {
	uint8_t x;
	for (x = 0; x < HIGHSCORES_TO_STORE; x++) {
		strcpy(highscore_names[x], "---");
		highscore_scores[x] = 0;
		highscore_levels[x] = 0;
	}
}

void load_highscores_eeprom(void) {
	if (test_signature()) {
		// Load names
		eeprom_read_block((void*)&highscore_names, (const void*)EEPROM_NAMES, EEPROM_NAMES_LENGTH);
		// Load scores
		eeprom_read_block((void*)&highscore_scores, (const void*)EEPROM_SCORES, EEPROM_SCORES_LENGTH);
		// Load levels
		eeprom_read_block((void*)&highscore_levels, (const void*)EEPROM_LEVELS, EEPROM_LEVELS_LENGTH);
		printf(highscore_names[0]);
	} else {
		prepare_signature();
		save_highscores_eeprom();
	}
}

void save_highscores_eeprom(void) {
	// Save names
	eeprom_update_block((const void*)&highscore_names, (void*)EEPROM_NAMES, EEPROM_NAMES_LENGTH);
	// Save scores
	eeprom_update_block((const void*)&highscore_scores, (void*)EEPROM_SCORES, EEPROM_SCORES_LENGTH);
	// Save levels
	eeprom_update_block((const void*)&highscore_levels, (void*)EEPROM_LEVELS, EEPROM_LEVELS_LENGTH);
}

// Finds an appropriate place to insert this score, shifting all others downwards.
int8_t get_appropriate_index(uint16_t score) {
	uint8_t x;
	for (x = 0; x < HIGHSCORES_TO_STORE; x++) {
		if (score >= highscore_scores[x]) {
			shift_values_after(x);
			return x;
		}
	}
	
	return -1;
}

void set_highscore(uint8_t index, const char* new_name, uint16_t new_score, uint8_t new_level) {
	strcpy(highscore_names[index], new_name);
	highscore_scores[index] = new_score;
	highscore_levels[index] = new_level;
}

const char* get_highscore_name(uint8_t index) {
	return highscore_names[index];
}

uint16_t get_highscore_score(uint8_t index) {
	return highscore_scores[index];
}

uint8_t get_highscore_level(uint8_t index) {
	return highscore_levels[index];
}


/* HELPER FUNCTIONS */
uint8_t test_signature(void) {
	char eeprom_string[6];
	uint8_t x;
	
	eeprom_read_block((void*)&eeprom_string, (const void*)EEPROM_SIGNATURE, EEPROM_SIGNATURE_LENGTH);
	
	for (x = 0; x < EEPROM_SIGNATURE_LENGTH; x++) {
		if (signature[x] != eeprom_string[x]) {
			return 0;
		}
	}
	return 1;
}

void prepare_signature(void) {
	eeprom_update_block((const void*)&signature, (void*)EEPROM_SIGNATURE, EEPROM_SIGNATURE_LENGTH);
}

void shift_values_after(uint8_t index_to_shift) {
	uint8_t x;
	for (x = HIGHSCORES_TO_STORE-1; x > index_to_shift; x--) {
		strcpy(highscore_names[x], highscore_names[x-1]);
		highscore_scores[x] = highscore_scores[x-1];
		highscore_levels[x] = highscore_levels[x-1];
	}
}