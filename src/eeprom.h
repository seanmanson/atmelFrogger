/*
 * eeprom.h
 *
 * Author: Sean Manson
 *
 * EEPROM code for loading and saving highscores.
 *
 * This game uses 128 bytes of EEPROM, starting at address 8:
 *    - 8 bytes of signature
 *    - 5 * 21 bytes for a name
 *    - 5 * 2 bytes for a score
 *    - 5 * 1 byte for a level
 *
 */ 


#ifndef EEPROM_H_
#define EEPROM_H_

#include <stdint.h>

#define HIGHSCORES_TO_STORE 5
#define HIGHSCORE_NAME_LENGTH 21

void init_highscores(void);
void load_highscores_eeprom(void);
void save_highscores_eeprom(void);

int8_t get_appropriate_index(uint16_t score);

void set_highscore(uint8_t index, const char* new_name, uint16_t new_score, uint8_t new_level);

const char* get_highscore_name(uint8_t index);
uint16_t get_highscore_score(uint8_t index);
uint8_t get_highscore_level(uint8_t index);

// Helper
uint8_t test_signature(void);
void prepare_signature(void);
void shift_values_after(uint8_t index_to_shift);

#endif /* EEPROM_H_ */