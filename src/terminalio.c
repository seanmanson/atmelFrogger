/*
 * terminalio.c
 *
 * Author: Peter Sutton
 * Edited: Sean Manson
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <avr/pgmspace.h>
#include "terminalio.h"

void move_cursor(int x, int y) {
    printf_P(PSTR("\x1b[%d;%dH"), y, x);
}

void normal_display_mode(void) {
	printf_P(PSTR("\x1b[0m"));
}

void reverse_video(void) {
	printf_P(PSTR("\x1b[7m"));
}

void clear_terminal(void) {
	printf_P(PSTR("\x1b[2J"));
}

void clear_to_end_of_line(void) {
	printf_P(PSTR("\x1b[K"));
}

void set_display_attribute(int8_t parameter) {
	printf_P(PSTR("\x1b[%dm"), parameter);
}

void draw_horizontal_line(int y, int startx, int endx) {
	int i;
	move_cursor(startx, y);
	reverse_video();
	for(i=startx; i <= endx; i++) {
		printf(" ");	/* No need to use printf_P - printing 
						 * a single character gets optimised
						 * to a putchar call 
						 */
	}
	normal_display_mode();
}

void draw_vertical_line(int x, int starty, int endy) {
	int i;
	move_cursor(x, starty);
	reverse_video();
	for(i=starty; i < endy; i++) {
		printf(" ");
		/* Move down one and back to the left one */
		printf_P(PSTR("\x1b[B\x1b[D"));
	}
	printf(" ");
	normal_display_mode();
}

void redraw_screen(void) {
	clear_terminal();
	set_display_attribute(0);
	draw_horizontal_line(SCREEN_TOP, SCREEN_LEFT, SCREEN_LEFT + SCREEN_WIDTH);
	draw_horizontal_line(SCREEN_TOP + SCREEN_HEIGHT, SCREEN_LEFT, SCREEN_LEFT + SCREEN_WIDTH);
	draw_vertical_line(SCREEN_LEFT, SCREEN_TOP, SCREEN_TOP + SCREEN_HEIGHT);
	draw_vertical_line(SCREEN_LEFT + SCREEN_WIDTH, SCREEN_TOP, SCREEN_TOP + SCREEN_HEIGHT);
}

void draw_rainbow_text(char string[], int x, int y) {
	uint8_t len = strlen(string);
	uint8_t i;
	uint8_t colour = RED_TEXT;
	for (i=0; i<len; i++) {
		move_cursor(x+i, y);
		set_display_attribute(colour);
		printf_P(PSTR("%c"), string[i]);
		colour++;
		
		// Fixes order to be as a rainbow
		if (colour == BLUE_TEXT) {
			colour = CYAN_TEXT;
		} else if (colour == WHITE_TEXT) {
			colour = MAGENTA_TEXT;
		} else if (colour == CYAN_TEXT) {
			colour = RED_TEXT;
		}
	}
	set_display_attribute(0);
}
