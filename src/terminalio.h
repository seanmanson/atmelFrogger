/*
 * terminalio.h
 *
 * Author: Peter Sutton
 * Edited: Sean Manson
 *
 * Functions for interacting with the terminal. These should be used
 * to encapsulate all sending of escape sequences.
 */

#ifndef TERMINAL_IO_H_
#define TERMINAL_IO_H_

#include <stdint.h>


#define SCREEN_TOP 2
#define SCREEN_LEFT 2
#define SCREEN_HEIGHT 22
#define SCREEN_WIDTH 60
#define SCREENSPACE(x, y) (SCREEN_LEFT+(x)),(SCREEN_TOP+(y))

#define RED_TEXT 31
#define GREEN_TEXT 32
#define YELLOW_TEXT 33
#define BLUE_TEXT 34
#define MAGENTA_TEXT 35
#define CYAN_TEXT 36
#define WHITE_TEXT 37

/*
 * x and y are measured relative to the top left of the screen. First
 * column is 1, first row is 1.
 *
 * The display parameter is a number between 0 and 47 (not all values
 * are valid).
 */

void move_cursor(int x, int y);
void normal_display_mode(void);
void reverse_video(void);
void clear_terminal(void);
void clear_to_end_of_line(void);
void set_display_attribute(int8_t parameter);

/*
 * Draw a reverse video line on the terminal. startx must be <= endx.
 * starty must be <= endy
 */
void draw_horizontal_line(int y, int startx, int endx);
void draw_vertical_line(int x, int starty, int endy);

/*
 * Draw the background screen for the game.
 */
void redraw_screen(void);

/*
 * Cool rainbows!
 * Draws the given string in a rainbow of colour starting from cursor point x, y.
 * This DOES change what the current cursor point is--you will have to change it
 * to something else yourself afterwards.
 */
void draw_rainbow_text(char string[], int x, int y);

#endif /* TERMINAL_IO_H */