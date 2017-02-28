/*
 * buttons.h
 *
 * Author: Peter Sutton
 * Edited: Sean Manson
 *
 * We assume four push buttons (B0 to B3) are connected to pins B0 to B3. We configure
 * pin change interrupts on these pins.
 *
 * These buttons can also be repeated at certain intervals after this interrupt fires
 * by being continually held down.
 */ 


#ifndef BUTTONS_H_
#define BUTTONS_H_

#include <stdint.h>

/* Set up pin change interrupts on pins B0 to B3.
 * This function disables and reenables interrupts as necessary.
 */
void init_button_interrupts(void);

/* Return the last button pushed (0 to 3) or -1 if there are no
 * button pushes to return. (A small queue of button pushes is
 * kept. This function should be called frequently enough to
 * ensure the queue does not overflow. Excess button pushes are
 * discarded.)
 */
int8_t button_pushed(void);

/* Activate/deactivate the button functionality as desired.
 */
void activate_buttons(void);
void deactivate_buttons(void);

/* Returns whether the last held arrangement of buttons should be repeated, taking
 * into account the time since the last button movement.
 *
 * The value returned is -1 if no buttons should repeat, and n if Bn should repeat.
 *
 * Note that to make sense, this function should be run more frequently than the delay
 * time between movements.
 */
int8_t should_button_repeat(void);

#endif /* BUTTONS_H_ */