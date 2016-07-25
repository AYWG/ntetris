#ifndef NTETRIS_H
#define NTETRIS_H

/* Libraries */
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

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
#define SPACE_KEY 32

/* Start menu options */
#define START 1
#define EXIT 2 

/* Difficulty levels */
#define CASUAL 1000000
#define INTERMEDIATE 600000
#define EXPERT 300000

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

#define WELL_L_BNDRY 1
#define WELL_R_BNDRY WELL_WIDTH - 2
#define WELL_T_BNDRY 1
#define WELL_B_BNDRY WELL_HEIGHT - 2
/*
// WELL_SIZE refers to the number of different coordinates
// for which tetriminos can exist in (i.e. all coordinates not
// including borders)
#define WELL_SIZE (WELL_HEIGHT - 2) * (WELL_WIDTH - 2)
*/
#define COVER_HEIGHT 3
#define COVER_WIDTH WELL_WIDTH
#define COVER_INIT_Y WELL_INIT_Y
#define COVER_INIT_X WELL_INIT_X
#define COVER_B_BNDRY COVER_INIT_Y + COVER_HEIGHT

#define HOLD_WIDTH
#define HOLD_HEIGHT
#define HOLD_INIT_Y WELL_INIT_Y
#define HOLD_INIT_X

#define LINE_COUNT_WIDTH
#define LINE_COUNT_HEIGHT
#define LINE_COUNT_INIT_Y
#define LINE_COUNT_INIT_X

#define SCORE_WIDTH
#define SCORE_HEIGHT
#define SCORE_INIT_Y
#define SCORE_INIT_X

#define ONE_SEC_DELAY 1000000 // microseconds
#define SMALL_DELAY 1000

typedef struct 
{
	int y, x;
	chtype value;
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

typedef struct
{
	WINDOW *win;
	TETRIMINO *tetrimino;
	int game_delay;
} THREAD_ARGS;


/* Function prototypes */
void ntetris_init ();
void print_title ();
void print_menu (WINDOW *menu_win, int highlight);
int get_menu_choice ();
void update_well(WINDOW *win, TETRIMINO *tetrimino);
void clear_well(WINDOW *win);

void move_tetrimino (WINDOW *win, TETRIMINO *tetrimino, int direction);
void get_rotated_bits(COORDINATE_PAIR pivot, COORDINATE_PAIR old_bits[], COORDINATE_PAIR new_bits[], int num_bits);
void rotate_tetrimino (WINDOW *win, TETRIMINO *tetrimino);
void drop_tetrimino (WINDOW *win, TETRIMINO *tetrimino);
void init_tetrimino (WINDOW *win, TETRIMINO *tetrimino, int tetrimino_id);
void lock_tetrimino_into_well(TETRIMINO *tetrimino);
int get_rand_tetrimino ();
void *play_ntetris (void *difficulty);
void *periodic_thread(void *arguments);
void *lock_in_thread(void *arguments);
int equal_coords (COORDINATE_PAIR cp_1, COORDINATE_PAIR cp_2);
int equal_bits (COORDINATE_PAIR bits_1[], COORDINATE_PAIR bits_2[], int num_bits);
void copy_bits (COORDINATE_PAIR source_bits[], COORDINATE_PAIR dest_bits[], int num_bits);
int out_of_boundaries (WINDOW *win, COORDINATE_PAIR coords);
int valid_position (WINDOW *win, TETRIMINO *tetrimino, COORDINATE_PAIR new_coords[], int num_new_coords);
int row_complete (int row);

extern int n_menu_choices;
extern COORDINATE_PAIR well_contents[WELL_HEIGHT - 2][WELL_WIDTH - 2];
#endif





