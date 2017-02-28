/*
 * joystick.h
 *
 * Author: Sean Manson
 *
 * Implements the joystick, with its associated interrupts and commands.
 * Should allow for diagonal movement.
 *
 * The analog joystick inputs should be connected to the A6/A7 ports, where A6
 * is X and A7 is Y.
 *
 * A joystick works by having its area split up into specific 'zones'.
 * The zones are shaped and numbered similar to:
 * _________________
 *|     |  1  |     |
 *|  0  |_____|  2  |
 *|_____|     |_____|
 *|   |         |   |
 *| 3 |    4    | 5 |
 *|___|_       _|___|
 *|     |_____|     |
 *|  6  |     |  8  |
 *|_____|__7__|_____|
 *
 * This allows for diagonal movement to be more easily activated.
 */ 


#ifndef JOYSTICK_H_
#define JOYSTICK_H_

#include <stdint.h>

// Zone numbers
#define TOPLEFT 0
#define TOP 1
#define TOPRIGHT 2
#define LEFT 3
#define CENTRE 4
#define RIGHT 5
#define BOTTOMLEFT 6
#define BOTTOM 7
#define BOTTOMRIGHT 8

// Height and width of this joystick
#define WIDTH 1024
#define HEIGHT 1024

// Center position
// This is calibrated for my joystick; I dunno if this is the same for all of them.
#define CENTRE_MID_X 515
#define CENTRE_MID_Y 545

// Dead zone values
// Given by the distances between the center and the edges (or the diagonal corner for corner zones)
#define DEAD_ZONE_X 40
#define DEAD_ZONE_Y 50
#define DEAD_ZONE_DIAG 31

/* Initialises the joystick hardware and sets it up to start converting joystick analog values
 * to digital ones.
 */
void init_joystick(void);

/* Returns whether or not the joystick is telling the player to move.
 * To be more specific, a joystick movement happens when the player either moves
 * from the center to one of the outside areas, or if they've been around the
 * outside for longer than an internal delay value.
 */
uint8_t should_joystick_move(void);

/* Returns the last zone of movement of the joystick, as calculated in the last method.
 * Should ONLY be run right after executing the previous method.
 */
uint8_t get_last_joystick_movement_value(void);

/* HELPER FUNCTIONS */
// Returns the last x/y value of the joystick
uint16_t get_last_x(void);
uint16_t get_last_y(void);

// Finds the current zone the joystick is in
uint8_t get_current_zone(void);

#endif /* JOYSTICK_H_ */