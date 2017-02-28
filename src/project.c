/*
 * FroggerProject.c
 *
 * Main file
 *
 * Author: Peter Sutton.
 * Heavily revised by Sean Manson.
 * Student ID: 42846413
 */ 

// Libraries
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>

// Project files
#include "eeprom.h"
#include "ledmatrix.h"
#include "scrolling_char_display.h"
#include "buttons.h"
#include "joystick.h"
#include "serialio.h"
#include "terminalio.h"
#include "sound.h"
#include "score.h"
#include "lives.h"
#include "level.h"
#include "timer0.h"
#include "game.h"

// Delay settings
#define F_CPU 8000000L
#include <util/delay.h>

// Function prototypes - these are defined below (after main()) in the order
// given here
void initialise_hardware(void);
void splash_screen(void);
void new_game(void);
void play_game(void);
void new_level(void);
void play_level(void);
void pause_game(void);
void level_up(void);
void handle_game_over(void);
void handle_out_of_time(void);
void handle_lose_life(void);
void update_status_screen(void);
void confirmation_screen_pause(void);
void get_user_typing(char string_to_get[], uint8_t screen_x, uint8_t screen_y);
uint8_t new_game_pressed(void);
uint8_t enter_pressed(void);
uint8_t new_game_or_enter_pressed(void);
uint8_t pause_pressed(void);

// ASCII code for Escape/Delete character
#define ESCAPE_CHAR 27
#define DELETE_CHAR 127

// Move times for each row, given as hundredths of seconds
#define BASE_SPEED_TRAFFIC_1 80
#define BASE_SPEED_TRAFFIC_2 60
#define BASE_SPEED_TRAFFIC_3 120
#define BASE_SPEED_LOGS_1 100
#define BASE_SPEED_LOGS_2 75

// Time permitted to get across to the other side
#define BASE_TIME_PER_FROG 25

// Flag for starting a new game 
// Needed in order for the game process to run correctly
static uint8_t new_game_flag = 0;


/////////////////////////////// main //////////////////////////////////
int main(void) {
	// Setup hardware and call backs. This will turn on 
	// interrupts.
	initialise_hardware();
	
	// Show the splash screen message. Returns when display
	// is complete
	play_tune_startup();
	init_scrolling_display();
	splash_screen();
	
	while(1) {
		// Start a new game
		new_game();
		new_game_flag = 0;
		
		// Play this game
		play_game();
		
		// If the game is actually over, show a game over screen
		if (!new_game_flag) {
			handle_game_over();
		}
	}
}

// Set registers for playing
void initialise_hardware(void) {
	// Ready highscores + Setup access to the EEPROM
	init_highscores();
	load_highscores_eeprom();
	
	ledmatrix_setup();
	init_button_interrupts();
	// Setup serial port for 19200 baud communication with no echo
	// of incoming characters
	init_serial_stdio(19200,0);
	
	// Setup 1ms timers
	init_timer0();
	
	// Setup buzzer
	init_buzzer();
	
	// Setup LED lives outputs
	init_lives_display();
	
	// Setup joystick
	init_joystick();
	
	// Turn on global interrupts
	sei();
}

// Opening splash screen
// Press button, 'n' or enter to continue
void splash_screen(void) {
	// Clear terminal screen
	redraw_screen();
	
	// Output greeting message
	draw_rainbow_text("Frogger - Xtended Edition", SCREENSPACE(10,2));
	set_display_attribute(GREEN_TEXT);
	move_cursor(SCREENSPACE(5,3));
	printf_P(PSTR("CSSE2010 project by Sean Manson (SID: 42846413)"));
	move_cursor(SCREENSPACE(20, 5));
	printf_P(PSTR("HIGHSCORES"));
	move_cursor(SCREENSPACE(6, 6));
	printf_P(PSTR("RANK"));
	move_cursor(SCREENSPACE(12, 6));
	printf_P(PSTR("NAME"));
	move_cursor(SCREENSPACE(32, 6));
	printf_P(PSTR("SCORE"));
	move_cursor(SCREENSPACE(39, 6));
	printf_P(PSTR("LEVEL"));
	uint8_t x;
	for (x = 0; x < HIGHSCORES_TO_STORE; x++) {
		move_cursor(SCREENSPACE(7, x+7));
		printf("%d", x+1);
		move_cursor(SCREENSPACE(11, x+7));
		printf(get_highscore_name(x));
		move_cursor(SCREENSPACE(34, x+7));
		printf("%d", get_highscore_score(x));
		move_cursor(SCREENSPACE(41, x+7));
		printf("%d", get_highscore_level(x));
	}
	move_cursor(SCREENSPACE(5,18));
	printf_P(PSTR("Press enter, 'n', or any button on the IO Board to"));
	move_cursor(SCREENSPACE(5,19));
	printf_P(PSTR("begin!"));
	
	// Output the scrolling message to the LED matrix
	// and wait for a push button, 'n' or enter to be pushed.
	ledmatrix_clear();
	set_text_colour(COLOUR_YELLOW);
	while(1) {
		set_scrolling_display_text("42846413 - Sean Manson - Frogger");
		// Scroll the message until it has scrolled off the 
		// display or a button is pushed or 'n' or enter is received
		while(scroll_display()) {
			_delay_ms(150);
			if(button_pushed() != -1 || new_game_or_enter_pressed()) {
				// Seed the random number generator based upon the time taken
				srand(get_clock_ticks());
				return;
			}
		}
	}
}

// Set up a new game from the beginning
// A single 'game' lasts until the player loses 
void new_game(void) {
	// Initialise the level at 1
	init_level();
	
	// Initialise the score at 0
	init_score();
	
	// Initialise the lives at 3
	init_lives();
	
	// Initialise the time at 0
	init_countdown();
}

// Play through the game, looping until the player loses
// Arranging play in this fashion ensures gameplay lasts forever
// and that we can check whenever a level is done
void play_game(void) {
	// Start the first level
	new_level();
	play_level();
	
	// If they have yet to lose and haven't started a new game, go to next level
	while(!player_has_lost() && !new_game_flag) {
		level_up();
		// If they try to start a new game while leveling up, break this cycle
		if (new_game_flag) {
			break;
		}
		new_level();
		play_level();
	}
}

// Start a new level
// A single 'level' lasts until either:
// All the frogs are safe,
// The player has pressed 'new game'
// The game ends for whatever reason.
void new_level(void) {
	// Initialise the game and display
	init_game();
	
	//prepare the initial status screen
	redraw_screen();
	update_status_screen();
	
	//Start the game clock
	start_ingame_timer();
	
	// Clear a button push or serial input if any are waiting
	// (The cast to void means the return value is ignored.)
	(void)button_pushed();
	clear_serial_input_buffer();
}

// Play through the level, looping until the player wins/loses
void play_level(void) {
	uint32_t current_time; //current time
	uint32_t last_move_times[5]; //move times for each lane
	uint16_t base_speed = 1000; //movement speed
	int8_t button;
	char serial_input, escape_sequence_char;
	uint8_t characters_into_escape_sequence = 0;
	
	// Get the current time and remember this as the last time the vehicles
	// and logs were moved.
	current_time = get_ingame_clock_ticks();
	uint8_t i;
	for (i=0;i<5;i++) {
		last_move_times[i] = current_time;
	}
	
	// While we still should be playing this level:
	while (!player_has_lost() && !is_riverbank_full()) {
		// Update and get ready for next frog
		update_status_screen();
		remove_dead_frogs();
			
		// Place a new frog at the start to begin this loop
		put_frog_at_start();
		
		// Start countdown timer
		countdown_set(BASE_TIME_PER_FROG);
		
		// Repeat as long as frog is alive/has not reached riverbank:
		while(is_frog_alive() && !frog_has_reached_riverbank()) {
			// Check if they have run out of time
			if (is_countdown_done()) {
				kill_frog();
			}
			
			// Scroll lanes and check for death
			current_time = get_ingame_clock_ticks();
			if (is_frog_alive() && !frog_has_reached_riverbank()) {
				//only move things while the frog's alive
				for (i=0;i<5;i++) {
					switch (i) {
						case 0:
						base_speed = BASE_SPEED_TRAFFIC_1;
						break;
						case 1:
						base_speed = BASE_SPEED_TRAFFIC_2;
						break;
						case 2:
						base_speed = BASE_SPEED_TRAFFIC_3;
						break;
						case 3:
						base_speed = BASE_SPEED_LOGS_1;
						break;
						case 4:
						base_speed = BASE_SPEED_LOGS_2;
						break;
					}
					if(current_time >= last_move_times[i] + (base_speed*(10000/get_difficulty())/10)) {
						// If it has been base_speed*10 milliseconssecond since the last movement,
						// move the row associated with this movement time.
						// We also adjust this 'speed' (actually distance) for the difficulty by multiplying
						// by 100/difficulty. We first multiply by 1000 then divide by 10 because this provides
						// greater accuracy when working with integer division on the AVR.
						// Alternate the direction of movement based upon the current level.
						
						switch(i) {
							case 0:
							scroll_lane(0, get_level_direction());
							break;
							case 1:
							scroll_lane(1, -get_level_direction());
							break;
							case 2:
							scroll_lane(2, get_level_direction());
							break;
							case 3:
							scroll_log_channel(0, -get_level_direction());
							break;
							case 4:
							scroll_log_channel(1, get_level_direction());
							break;
						}
						last_move_times[i] = current_time;
					}
				}
			}
			
			// Check for input - which could be a button push or serial input.
			// Serial input may be part of an escape sequence, e.g. ESC [ D
			// is a left cursor key press. At most one of the following three
			// variables will be set to a value other than -1 if input is available.
			// (We don't initalise button to -1 since button_pushed() will return -1
			// if no button pushes are waiting to be returned.)
			// Button pushes take priority over serial input. If there are both then
			// we'll retrieve the serial input the next time through this loop
			serial_input = -1;
			escape_sequence_char = -1;
			button = button_pushed();
		
			if(button == -1) {
				// No push button was pushed, see if there is any serial input
				while(serial_input_available()) {
					// Serial data was available - read the data from standard input
					serial_input = fgetc(stdin);
					// Check if the character is part of an escape sequence
					if(characters_into_escape_sequence == 0 && serial_input == ESCAPE_CHAR) {
						// We've hit the first character in an escape sequence (escape)
						characters_into_escape_sequence++;
						serial_input = -1; // Don't further process this character
					} else if(characters_into_escape_sequence == 1 && serial_input == '[') {
						// We've hit the second character in an escape sequence
						characters_into_escape_sequence++;
						serial_input = -1; // Don't further process this character
					} else if(characters_into_escape_sequence == 2) {
						// Third (and last) character in the escape sequence
						escape_sequence_char = serial_input;
						serial_input = -1;  // Don't further process this character - we
											// deal with it as part of the escape sequence
						characters_into_escape_sequence = 0;
					} else {
						// Character was not part of an escape sequence (or we received
						// an invalid second character in the sequence). We'll process 
						// the data in the serial_input variable.
						characters_into_escape_sequence = 0;
					}
				}
			}
		
			// Process the input. 
			if(button==3 || escape_sequence_char=='D' || serial_input=='L' || serial_input=='l') {
				// Attempt to move left
				move_frog_left();
			} else if(button==2 || escape_sequence_char=='A' || serial_input=='U' || serial_input=='u') {
				// Attempt to move forward
				move_frog_forward();
			} else if(button==1 || escape_sequence_char=='B' || serial_input=='D' || serial_input=='d') {
				// Attempt to move down
				move_frog_backward();
			} else if(button==0 || escape_sequence_char=='C' || serial_input=='R' || serial_input=='r') {
				// Attempt to move right
				move_frog_right();
			} else if(serial_input == 'n' || serial_input == 'N') {
				// Start new game
				new_game_flag = 1;
				return; // Quits out of the play_game() function
			} else if(serial_input == 'p' || serial_input == 'P') {
				// Pause game
				pause_game();
			} else if (should_joystick_move()) {
				// If the joystick is telling us we should move,
				// Go through all the movement options and attempt to move accordingly
				switch (get_last_joystick_movement_value()) {
					case TOPLEFT:
					move_frog_forward_left();
					break;
					case TOP:
					move_frog_forward();
					break;
					case TOPRIGHT:
					move_frog_forward_right();
					break;
					case LEFT:
					move_frog_left();
					break;
					case RIGHT:
					move_frog_right();
					break;
					case BOTTOMLEFT:
					move_frog_backward_left();
					break;
					case BOTTOM:
					move_frog_backward();
					break;
					case BOTTOMRIGHT:
					move_frog_backward_right();
					break;
				}
			} else { 
				// Auto-repeated button inputs
				switch (should_button_repeat()) {
					case 3:
						move_frog_left();
						break;
					case 2:
						move_frog_forward();
						break;
					case 1:
						move_frog_backward();
						break;
					case 0:
						move_frog_right();
						break;
				}
			}
		}
		
		// We get here when this frog's time is over
		// They have either gotten to the other side or died
		
		if(is_frog_alive() && frog_has_reached_riverbank()) {
			// Add to their score for making it to the other side
			// This score is the base score + the time they have remaining
			play_tune_success();
			add_to_score(BASE_SCORE_GET_TO_RIVERBANK + get_countdown_time_remaining());
			countdown_clear();
		} else if (is_countdown_done()) {
			// If they have run out of time,
			// tell them so, and make them lose a life
			handle_out_of_time();
		} else {
			// If the frog is dead,
			// clear the countdown, tell them so, and make them lose a life
			countdown_clear();
			handle_lose_life();
		}
		
		// If they pressed 'n' in one of the message windows, start new game
		if (new_game_flag == 1) {
			return;
		}
	}
	
	// We get here if all lives are spent or the riverbank is complete
	
	// Give them points if they completed this level
	if(is_riverbank_full()) {
		add_to_score(BASE_SCORE_COMPLETE_LEVEL);
		update_status_screen();
	}
	
	// Repeat level if we should
}

// Increment the current level, showing appropriate effects
void level_up() {
	increment_level();
	flip_level_direction();
	
	// Output the scrolling message to the LED matrix
	// and wait for a push button to be pushed.
	ledmatrix_clear();
	
	// Refresh the terminal and display an appropriate message
	redraw_screen();
	set_display_attribute(GREEN_TEXT);
	move_cursor(SCREENSPACE(10, 7));
	printf_P(PSTR("Level up!"));
	move_cursor(SCREENSPACE(5, 10));
	printf_P(PSTR("You are now on level %d."), get_level());
	move_cursor(SCREENSPACE(5, 11));
	printf_P(PSTR("The game is now running at %d.%02dx speed."), get_factor_ones(), get_factor_tenthshundreths());
	
	if (!get_at_max_lives()) {
		move_cursor(SCREENSPACE(5, 13));
		printf_P(PSTR("You gain a life!"));
		gain_life();
	}
	
	// Generate a string for the new level name
	char level_name[10];
	snprintf(level_name, 10, "Level %d", get_level());
	
	// Scroll this screen on the LEDs
	init_scrolling_display();
	set_text_colour(COLOUR_GREEN);
	set_scrolling_display_text(level_name);
	while(scroll_display()) {
		if (new_game_pressed()) {
			new_game_flag = 1;
			clear_serial_input_buffer();
			break;
		}
		_delay_ms(100);
	}
}

// Pause the game
// Only runs when 'p' is pressed during gameplay
void pause_game() {
	// Stop the clock from counting
	stop_ingame_timer();
	
	// Deactivate button interrupts
	deactivate_buttons();
	
	// Refresh the terminal and display a message
	redraw_screen();
	set_display_attribute(GREEN_TEXT);
	move_cursor(SCREENSPACE(10, 7));
	printf_P(PSTR("PAUSED"));
	move_cursor(SCREENSPACE(5, 9));
	printf_P(PSTR("Game is currently paused."));
	move_cursor(SCREENSPACE(5, 10));
	printf_P(PSTR("All inputs are being ignored."));
	move_cursor(SCREENSPACE(5, 11));
	printf_P(PSTR("Press 'p' to continue."));
	
	// Wait until they press 'p' again
	while(!pause_pressed()) {
		; // wait
	}
	clear_serial_input_buffer();
	
	// Redraw game to serial output because it would have been cleared
	redraw_screen();
	update_status_screen();
	
	// Reactivate button interrupts
	activate_buttons();
	
	// Restart the game clock
	start_ingame_timer();
}

// Confirmation when they run out of time
void handle_out_of_time() {
	lose_life();
	
	// If we aren't dead
	if (!player_has_lost()) { 
		play_tune_dead(); // Play death jingle
		
		// Stop the game clock
		stop_ingame_timer();
		
		// Give an appropriate message
		redraw_screen();
		set_display_attribute(GREEN_TEXT);
		move_cursor(SCREENSPACE(10, 7));
		printf_P(PSTR("You ran out of time!"));
		move_cursor(SCREENSPACE(10, 8));
		printf_P(PSTR("You have lost a life."));
		move_cursor(SCREENSPACE(7, 10));
		if (get_lives() == 1) {
			printf_P(PSTR("You now have 1 life remaining."));
		} else {
			printf_P(PSTR("You now have %d lives remaining."), get_lives());
		}
		if (get_lives() == 0) {
			move_cursor(SCREENSPACE(10, 11));
			printf_P(PSTR("Watch out!"));
		}
		move_cursor(SCREENSPACE(5, 13));
		printf_P(PSTR("Press enter or any button on the IO Board"));
		move_cursor(SCREENSPACE(5, 14));
		printf_P(PSTR("to continue..."));
		
		// Wait for them to respond with enter or 'n'
		confirmation_screen_pause();
		redraw_screen();
		
		// Restart the clock
		start_ingame_timer();
	} else {
		play_tune_lost(); // Play lost game jingle
	}
}

// Confirmation when they lose a life
void handle_lose_life() {
	lose_life();
	
	// If we aren't dead
	if (!player_has_lost()) {
		play_tune_dead(); // Play death jingle
		
		// Stop the game clock
		stop_ingame_timer();
		
		// Give an appropriate message
		redraw_screen();
		set_display_attribute(GREEN_TEXT);
		move_cursor(SCREENSPACE(10, 7));
		printf_P(PSTR("You lost a life!"));
		move_cursor(SCREENSPACE(7, 9));
		if (get_lives() == 1) {
			printf_P(PSTR("You now have 1 life remaining."));
		} else {
			printf_P(PSTR("You now have %d lives remaining."), get_lives());
		}
		if (get_lives() == 0) {
			move_cursor(SCREENSPACE(10, 10));
			printf_P(PSTR("Watch out!"));
		}
		move_cursor(SCREENSPACE(5, 12));
		printf_P(PSTR("Press enter or any button on the IO Board "));
		move_cursor(SCREENSPACE(5, 13));
		printf_P(PSTR("to continue..."));
		
		// Wait for them to respond with enter or 'n'
		confirmation_screen_pause();
		redraw_screen();
		
		// Restart the clock
		start_ingame_timer();
	} else {
		play_tune_lost(); // Play lost game jingle
	}
}

// Confirmation when they lose
void handle_game_over() {
	uint8_t x;
	int8_t should_type;
	char new_highscore_name[HIGHSCORE_NAME_LENGTH+1] = "";
	// Display an appropriate message
	redraw_screen();
	set_display_attribute(GREEN_TEXT);
	
	move_cursor(SCREENSPACE(10, 2));
	printf_P(PSTR("GAME OVER!"));
	move_cursor(SCREENSPACE(5, 3));
	printf_P(PSTR("Your score was %d, and you made it to level %d!"), get_score(), get_level());
	
	should_type = get_appropriate_index(get_score());
	
	move_cursor(SCREENSPACE(20, 5));
	printf_P(PSTR("HIGHSCORES"));
	move_cursor(SCREENSPACE(6, 6));
	printf_P(PSTR("RANK"));
	move_cursor(SCREENSPACE(12, 6));
	printf_P(PSTR("NAME"));
	move_cursor(SCREENSPACE(32, 6));
	printf_P(PSTR("SCORE"));
	move_cursor(SCREENSPACE(39, 6));
	printf_P(PSTR("LEVEL"));
	for (x = 0; x < HIGHSCORES_TO_STORE; x++) {
		move_cursor(SCREENSPACE(7, x+7));
		printf("%d", x+1);
		move_cursor(SCREENSPACE(11, x+7));
		printf(get_highscore_name(x));
		move_cursor(SCREENSPACE(34, x+7));
		printf("%d", get_highscore_score(x));
		move_cursor(SCREENSPACE(41, x+7));
		printf("%d", get_highscore_level(x));
	}
	
	if (should_type != -1) {
		move_cursor(SCREENSPACE(5, 18));
		printf_P(PSTR("You obtained a high score!"));
		move_cursor(SCREENSPACE(5, 19));
		printf_P(PSTR("Please type your name (max 20 chars) above."));
		
		// Get the user's response
		get_user_typing(new_highscore_name, 11, should_type+7);
		
		// Save to highscores
		set_highscore(should_type, new_highscore_name, get_score(), get_level());
		
		// Refresh line of highscores
		set_display_attribute(GREEN_TEXT);
		move_cursor(SCREENSPACE(11, should_type+7));
		for (x=0; x<23; x++) {
			printf(" ");
		}	
		move_cursor(SCREENSPACE(11, should_type+7));
		printf(get_highscore_name(should_type));
		move_cursor(SCREENSPACE(34, should_type+7));
		printf("%d", get_highscore_score(should_type));
		move_cursor(SCREENSPACE(41, should_type+7));
		printf("%d", get_highscore_level(should_type));
		
		// Save to eeprom
		save_highscores_eeprom();
		
		// Clear bottom message
		move_cursor(SCREENSPACE(5, 18));
		for (x=0; x<53; x++) {
			printf(" ");
		}
		move_cursor(SCREENSPACE(5, 19));
		for (x=0; x<53; x++) {
			printf(" ");
		}
	}
	
	move_cursor(SCREENSPACE(5, 18));
	printf_P(PSTR("Press enter, 'n' (or any button on the IO Board) to"));
	move_cursor(SCREENSPACE(5, 19));
	printf_P(PSTR("start a new game..."));
	
	// Wait for them to respond with enter or 'n'
	confirmation_screen_pause();
}


/* HELPER/SECONDARY FUNCTIONS */
// Update the in-game terminal status
void update_status_screen() {
	set_display_attribute(GREEN_TEXT);
	move_cursor(SCREENSPACE(10, 7));
	printf_P(PSTR("GAME IN PROGRESS..."));
	move_cursor(SCREENSPACE(5, 9));
	printf_P(PSTR("Current Level: %d"), get_level());
	move_cursor(SCREENSPACE(5, 10));
	printf_P(PSTR("Current Speed: %d.%02dx"), get_factor_ones(), get_factor_tenthshundreths());
	move_cursor(SCREENSPACE(5, 11));
	printf_P(PSTR("Current Score: %d"), get_score());
	move_cursor(SCREENSPACE(5, 12));
	printf_P(PSTR("Current Lives: %d"), get_lives());
}

// Pause and wait until they either push a button, enter or 'n'
void confirmation_screen_pause() {
	char serial_input;
	// Wait for button pushed
	while(button_pushed() == -1) {
		// If they input something over the terminal:
		if (serial_input_available()) {
			serial_input = fgetc(stdin);
			if (serial_input == 'n' || serial_input == 'N') { // New game
				new_game_flag = 1;
				break; // Exits this while loop
			} else if (serial_input == '\n' || serial_input == '\r') { // Continue
				break; // Exits this while loop
			}
		}
	}
	clear_serial_input_buffer();
}

void get_user_typing(char string_to_get[], uint8_t screen_x, uint8_t screen_y) {
	// Define our string
	uint8_t done = 0, current_pos = 0, x;
	char serial_input;
	
	while (done == 0) {
		// Draw what they've typed so far
		set_display_attribute(GREEN_TEXT);
		move_cursor(SCREENSPACE(screen_x, screen_y));
		printf(string_to_get);
		reverse_video();
		printf(" ");
		normal_display_mode();
		for (x = 0; x<(HIGHSCORE_NAME_LENGTH-current_pos); x++) {
			printf(" ");
		}
		move_cursor(SCREENSPACE(screen_x+current_pos, screen_y));
		
		// Wait for serial input
		while (!serial_input_available()) {
			;
		}
		// Break down this input
		serial_input = fgetc(stdin);
		printf("%c",serial_input);
		if (serial_input != ESCAPE_CHAR) { //ignore escape sequences
			if (serial_input == '\n' || serial_input == '\r') { // Enter
				if (current_pos != 0) { // If they have entered something
					done = 1;
				}
			} else if (serial_input == '\b' || serial_input == DELETE_CHAR) { // Backspace
				if (current_pos != 0) { // If they have entered something
					current_pos--;
					string_to_get[current_pos] = 0;
				}
			} else if (serial_input >= ' ' && serial_input <= '~') {// All printable characters
				if (current_pos < HIGHSCORE_NAME_LENGTH) {
					string_to_get[current_pos] = serial_input;
					current_pos++;
				}
			} 
		}
		clear_serial_input_buffer();
	}
}

// Returns whether new game button has been pressed last
uint8_t new_game_pressed() {
	if (!serial_input_available()) {
		return 0;
	}
	char serial_input = fgetc(stdin);
	return (serial_input == 'n' || serial_input == 'N');
}

// Returns whether enter has been pressed last
uint8_t enter_pressed() {
	if (!serial_input_available()) {
		return 0;
	}
	char serial_input = fgetc(stdin);
	return (serial_input == '\n' || serial_input == '\r');
}

// Returns whether new game button or enter has been pressed last
uint8_t new_game_or_enter_pressed() {
	if (!serial_input_available()) {
		return 0;
	}
	char serial_input = fgetc(stdin);
	return (serial_input == 'n' || serial_input == 'N' || serial_input == '\n' || serial_input == '\r');
}

// Returns whether 'p' has been pressed last
uint8_t pause_pressed() {
	if (!serial_input_available()) {
		return 0;
	}
	char serial_input = fgetc(stdin);
	return (serial_input == 'p' || serial_input == 'P');
}