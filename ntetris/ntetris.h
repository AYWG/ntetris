#ifndef NTETRIS_H
#define NTETRIS_H

/* Libraries */
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define TITLE_COLOR_PAIR 1
#define MAIN_TEXT_COLOR_PAIR 2 
#define I_COLOR_PAIR 3
#define J_COLOR_PAIR 4
#define L_COLOR_PAIR 5
#define O_COLOR_PAIR 6
#define S_COLOR_PAIR 7
#define T_COLOR_PAIR 8
#define Z_COLOR_PAIR 9

#define COLOR_ORANGE 8

#define MENU_HEIGHT 5
#define MENU_WIDTH 15


#define ENTER_KEY 10
#define QUIT_KEY 113

/* Start menu options */
#define START 1
#define EXIT 2 

/* Difficulty levels */
#define CASUAL 0
#define INTERMEDIATE 1
#define EXPERT 2

/* IDs of the different game pieces */
#define TETRIMINO_I 0
#define TETRIMINO_J 1
#define TETRIMINO_L 2
#define TETRIMINO_O 3 
#define TETRIMINO_S 4
#define TETRIMINO_T 5
#define TETRIMINO_Z 6

#define NUM_BITS 4

/* Dimensions and initial coordinates for 
the WINDOWs used */
#define WELL_HEIGHT 23
#define WELL_WIDTH 26
#define WELL_INIT_Y 0
#define WELL_INIT_X 26

#define COVER_HEIGHT 3
#define COVER_WIDTH WELL_WIDTH
#define COVER_INIT_Y WELL_INIT_Y
#define COVER_INIT_X WELL_INIT_X

#define HOLD_WIDTH
#define HOLD_HEIGHT
#define HOLD_INIT_Y
#define HOLD_INIT_X

#define LINE_COUNT_WIDTH
#define LINE_COUNT_HEIGHT
#define LINE_COUNT_INIT_Y
#define LINE_COUNT_INIT_X

#define SCORE_WIDTH
#define SCORE_HEIGHT
#define SCORE_INIT_Y
#define SCORE_INIT_X


#define SMALL_DELAY 100
#define ONE_SEC_DELAY 1000000

typedef struct 
{
	int y, x;
} COORDINATE_PAIR;


/* Struct representing the game pieces*/
typedef struct 
{
	/* Which tetrimino is it? */
	int tetrimino_type;

	/* The bit that is used as pivot when the tetrimino rotates */
	int pivot_bit;

	/* The coordinates of each of
	the bits that make up the tetrimino*/
	COORDINATE_PAIR bits[NUM_BITS];
} TETRIMINO;


/* Function prototypes */
void ntetris_init ();
void print_title ();
void print_menu (WINDOW *menu_win, int highlight);
int get_menu_choice ();


void move_tetrimino (WINDOW *win, TETRIMINO *tetrimino, int direction);
TETRIMINO *copy_tetrimino (TETRIMINO *tetrimino);
void rotate_tetrimino (WINDOW *win, TETRIMINO *tetrimino);
void drop_tetrimino (WINDOW *win, TETRIMINO *tetrimino);
void init_tetrimino (WINDOW *win, TETRIMINO *tetrimino, int tetrimino_id);
int get_rand_tetrimino ();
void play_ntetris (int difficulty);
int check_equal_coords (COORDINATE_PAIR cp_1, COORDINATE_PAIR cp_2);
int out_of_boundaries (WINDOW *win, COORDINATE_PAIR coords);
int valid_position (WINDOW *win, TETRIMINO *tetrimino, COORDINATE_PAIR new_coords[], int num_new_coords);

#endif





