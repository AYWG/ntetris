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

#define MENU_HEIGHT 7
#define MENU_WIDTH 20


#define ENTER_KEY 10
#define QUIT_KEY 113
#define SPACE_KEY 32
#define HOLD_KEY 122

/* Start menu options */
#define START 1
#define EXIT 2 

/* Difficulty levels */
#define CASUAL 1000000
#define INTERMEDIATE 500000
#define EXPERT 250000

/* IDs of the different game pieces */
#define TETRIMINO_I 0
#define TETRIMINO_J 1
#define TETRIMINO_L 2
#define TETRIMINO_O 3 
#define TETRIMINO_S 4
#define TETRIMINO_T 5
#define TETRIMINO_Z 6
#define INVALID_ID 7

#define NUM_BITS 4

#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4

/* Dimensions and initial coordinates for 
the WINDOWs used */
#define WELL_HEIGHT 23
#define WELL_WIDTH 22
#define WELL_INIT_Y 0
#define WELL_INIT_X 30

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

#define HOLD_WIDTH 8
#define HOLD_HEIGHT 6
#define HOLD_INIT_Y WELL_INIT_Y + COVER_HEIGHT - 1
#define HOLD_INIT_X WELL_INIT_X - 10

#define HOLD_L_BNDRY 1
#define HOLD_R_BNDRY HOLD_WIDTH - 2
#define HOLD_T_BNDRY 1
#define HOLD_B_BNDRY HOLD_HEIGHT - 2


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

#define ADJUST_LIMIT 20

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
void print_menu (WINDOW *menu_win, int highlight, char *menu_choices[], int num_menu_choices);
int get_menu_choice (char *menu_choices[], int num_menu_choices);
void draw_well(WINDOW *win, TETRIMINO *tetrimino);
void clear_well(WINDOW *win);
int update_hold(WINDOW *win, int tetrimino_id);

void move_tetrimino (WINDOW *win, TETRIMINO *tetrimino, int direction);
void get_rotated_bits(COORDINATE_PAIR pivot, COORDINATE_PAIR old_bits[], COORDINATE_PAIR new_bits[], int num_bits);
void rotate_tetrimino (WINDOW *win, TETRIMINO *tetrimino);
void drop_tetrimino (WINDOW *win, TETRIMINO *tetrimino, int game_delay);
void init_tetrimino (WINDOW *win, TETRIMINO *tetrimino, int tetrimino_id);
void lock_tetrimino_into_well(TETRIMINO *tetrimino);
void hold_tetrimino(WINDOW *well_win, WINDOW *hold_win, TETRIMINO *tetrimino);
int get_rand_num (int lower, int upper);
void *play_ntetris (void *difficulty);
void *periodic_thread(void *arguments);
void *lock_in_thread(void *arguments);
int equal_coords (COORDINATE_PAIR cp_1, COORDINATE_PAIR cp_2);
int equal_bits (COORDINATE_PAIR bits_1[], COORDINATE_PAIR bits_2[], int num_bits);
void copy_bits (COORDINATE_PAIR source_bits[], COORDINATE_PAIR dest_bits[], int num_bits);
int out_of_boundaries (WINDOW *win, COORDINATE_PAIR coords);
int valid_position (WINDOW *win, TETRIMINO *tetrimino, COORDINATE_PAIR new_bits[], int num_bits);
int row_complete (int row);
void clear_row (int row);
void update_well(WINDOW *win, TETRIMINO *tetrimino, int game_delay);

//extern int n_menu_choices;
//extern char *start_menu_choices[];
//extern char *difficulty_menu_choices[];
extern int RECENT_HOLD;
extern int CURRENTLY_HELD_TETRIMINO_ID;
extern COORDINATE_PAIR well_contents[WELL_HEIGHT - 2][WELL_WIDTH - 2];
#endif





