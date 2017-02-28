/*
 * sound.h
 * 
 * Author: Sean Manson
 *
 * Buzzer and sound controls.
 * 
 * This file contains setup for using the buzzer, as well as
 * controls for playing sound and jingles.
 *
 * A sound is a combination of a 16-bit frequency and an 8-bit
 * time value. The frequency is the Hz of the sound, while the
 * time is the length of the sound to play in hundredths of seconds.
 *
 * A jingle or tune is a combination of sounds in a particular order.
 * By playing multiple sounds in a row, this file can store them in
 * a buffer and play them in order as soon as each time value clears
 * up. This allows for the playing of simple melodies.
 *
 * Internally it uses timer1 for playing different frequencies and 
 * timer0 for organising its times. It stores an internal buffer of
 * the last 6 sounds, which it plays through at the soonest possible
 * opportunity.
 *
 * The buzzer must have one end set up at pin D5, and the other at
 * GND. To make life easy, this file internally sets up pin D7 to
 * take on the value of GND, which should make it easier to wire up.
 *
 * Sound can be muted using S7 on the IO board, which should be
 * wired to D2. Note that this does NOT internally deactivate the sound
 * registers from running -- the clock is still running, but the
 * output on D5 is being suppressed.
 *
 * Note that these functions rely heavily on timer0. Deactivating
 * timer0 (the whole clock, not the in-game copy) WILL result in
 * funny business with the timing of sounds.
 */

#ifndef SOUND_H_
#define SOUND_H_

#include <stdint.h>

// Define sound frequencies
#define FREQ_C4 262
#define FREQ_C4SHARP 277
#define FREQ_D4 294
#define FREQ_D4SHARP 311
#define FREQ_E4 330
#define FREQ_F4 349
#define FREQ_F4SHARP 370
#define FREQ_G4 392
#define FREQ_G4SHARP 415
#define FREQ_A4 440
#define FREQ_A4SHARP 466
#define FREQ_B4 494
#define FREQ_C5 523
#define FREQ_C5SHARP 554
#define FREQ_D5 587
#define FREQ_D5SHARP 622
#define FREQ_E5 659
#define FREQ_F5 698
#define FREQ_F5SHARP 740
#define FREQ_G5 784
#define FREQ_G5SHARP 831
#define FREQ_A5 880
#define FREQ_A5SHARP 932
#define FREQ_B5 988
#define FREQ_C6 1047

/* Sets up this buzzer for use on an IO level.
 */
void init_buzzer(void);

/* Add a new sound to the internal buffer. If the buffer
 * is full, or if the frequency is too low or high, this
 * command does nothing.
 */
void play_sound(uint16_t frequency, uint8_t time);

/* Add a new sound to the internal buffer ONLY if the
 * buffer is empty. Otherwise same as play_sound().
 * This is ideal for short sounds which should be ignored
 * when more important sounds are playing.
 */
void play_quiet_sound(uint16_t frequency, uint8_t time);

/* Clears all sounds in the buffer. Will cease all sounds
 * immediately after being run.
 */
void clear_sounds(void);

/* Returns whether or not a sound is contained in the buffer,
 * about to be played.
 */
uint8_t is_playing_sound(void);

/* HELPER FUNCTIONS */
/* Returns whether the D3 pin is on or not.
 */
uint8_t is_sound_on(void);

/* Returns the appropriate OCRB register value for the given
 * frequency.
 */
uint16_t get_OCRB_value(uint16_t frequency);

/* Advances to the next sound in the queue, setting the timer
 * accordingly.
 */
void start_next_sound(void);

/* Set pin D5 to start toggling values into the buzzer to play
 * sound.
 */
void start_toggling(void);

/* Set pin D5 to stop toggling values into the buzzer to play
 * sound.
 */
void stop_toggling(void);

/* PREDEFINED TUNES */
/* The following are some simple tunes which can be used at
 * various stages of the game.
 */
void play_tune_startup(void);
void play_tune_success(void);
void play_tune_dead(void);
void play_tune_lost(void);

#endif /* SOUND_H_ */