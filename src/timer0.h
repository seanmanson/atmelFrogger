/*
 * timer0.h
 *
 * Author: Peter Sutton
 * Edited by Sean Manson
 *
 * We set up timer 0 to give us an interrupt
 * every millisecond. Tasks that have to occur
 * regularly (every millisecond or few) can be added 
 * to the interrupt handler (in timer0.c) or can
 * be added to the main event loop that checks the
 * clock tick value. This value (32 bits) can be 
 * obtained using the get_clock_ticks() function.
 * (Any tasks undertaken in the interrupt handler
 * should be kept short so that we don't run the 
 * risk of missing an interrupt in future.)
 *
 * This file also contains the timer code for handling
 * the countdown. This outputs seven segment display
 * data to the C port pins, with a CC switcher at pin D2.
 */

#ifndef TIMER0_H_
#define TIMER0_H_

#include <stdint.h>

/* Set up our timer to give us an interrupt every millisecond
 * and update our time reference.
 */
void init_timer0(void);

/* Return the current clock tick value - milliseconds since the timer was
 * initialised.
 */
uint32_t get_clock_ticks(void);

/* Return the click value for the in-game timer, which can be manually
 * stopped and started.
 */
uint32_t get_ingame_clock_ticks(void);

/* Start the in-game timer.
 */
void start_ingame_timer(void);

/* Stop the in-game timer.
 */
void stop_ingame_timer(void);

/* Countdown functions */
/* Initialises the countdown's IO for use, setting it to 0.
 * The countdown automatically starts to count as soon as the internal value is
 * greater than 0. Note that when it does reach 0 it displays nothing.
 *
 * Note that the counter is based on the in-game timer, and so deactivating that will
 * stop the countdown and vice versa.
 */
void init_countdown(void);

/* Sets the counter's internal value, from which it automatically starts counting down.
 * Start time is given in seconds, but stored internally as millis.
 */
void countdown_set(uint8_t start);

/* Clears the internal counter value. (Sets it to 0). Does NOT deinitialise it.
 */
void countdown_clear(void);

/* Returns whether the countdown is currently 0 or not.
 */
uint8_t is_countdown_done(void);

/* Returns the number of seconds left in this countdown.
 */
uint8_t get_countdown_time_remaining(void);

#endif