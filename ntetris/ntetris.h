#ifndef NTETRIS_H
#define NTETRIS_H

/* Libraries */
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

/* IDs of the color pairs used in ntetris */
#define TITLE_COLOR_PAIR 1
#define MAIN_TEXT_COLOR_PAIR 2 // Default text color
#define I_COLOR_PAIR 3
#define J_COLOR_PAIR 4
#define L_COLOR_PAIR 5
#define O_COLOR_PAIR 6
#define S_COLOR_PAIR 7
#define T_COLOR_PAIR 8
#define Z_COLOR_PAIR 9

#define COLOR_ORANGE 8

/* Start menu options */
#define SINGLE 0
#define VERSUS 1
#define CONTROLS 2
#define EXIT 3

/* Difficulty levels */
#define CASUAL 0
#define INTERMEDIATE 1
#define EXPERT 2
#define BACK 3
#define INVALID_DIFF 4

/* Game delays (in microseconds) */
#define CASUAL_INIT_DELAY 1000000
#define INTERMEDIATE_INIT_DELAY 500000
#define EXPERT_INIT_DELAY 250000
#define MIN_DELAY 30000 // Game delay cannot go below this amount
#define STALL 1000

/* IDs of the different game pieces */
#define TETRIMINO_I 0
#define TETRIMINO_J 1
#define TETRIMINO_L 2
#define TETRIMINO_O 3 
#define TETRIMINO_S 4
#define TETRIMINO_T 5
#define TETRIMINO_Z 6
#define INVALID_ID 7

/* FOR REFERENCE
I : oooo - bits: 0 1 2 3  Pivot = 1

J : ooo  - bits: 0 1 2    Pivot = 1
  	  o 			 3

L : ooo  - bits: 0 1 2    Pivot = 1
	o 			 3

O : oo 	 - bits: 0 1  	  Pivot = 4 (no pivot)
	oo 			 2 3

S :  oo  - bits:  0 1     Pivot = 3
	oo 			2 3

T : ooo  - bits: 0 1 2 	  Pivot = 1
	 o 			   3

Z : oo 	 - bits: 0 1 	  Pivot = 2
	 oo 		   2 3
*/

/* Number of bits that make up each tetrimino */
#define NUM_BITS 4

/* Directions of movement */
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3

/* Directions of rotation */
#define CLOCKWISE 4
#define CNT_CLOCKWISE 5

/* Dimensions and initial coordinates for 
the WINDOWs used */
#define NUM_WINDOWS 7

#define WELL_ID 0

#define WELL_HEIGHT 23
#define WELL_WIDTH 22
#define WELL_INIT_Y 0
#define WELL_INIT_X 30

#define WELL_INIT_Y_P1 0
#define WELL_INIT_X_P1 46
#define WELL_INIT_Y_P2 0
#define WELL_INIT_X_P2 12

#define WELL_L_BNDRY 1
#define WELL_R_BNDRY WELL_WIDTH - 2
#define WELL_T_BNDRY 1
#define WELL_B_BNDRY WELL_HEIGHT - 2

#define WELL_CONTENTS_HEIGHT WELL_HEIGHT - 2
#define WELL_CONTENTS_WIDTH WELL_WIDTH - 2

#define COVER_ID 1

#define COVER_HEIGHT 3
#define COVER_WIDTH WELL_WIDTH
#define COVER_INIT_Y WELL_INIT_Y
#define COVER_INIT_X WELL_INIT_X

#define COVER_INIT_Y_P1 WELL_INIT_Y_P1
#define COVER_INIT_X_P1 WELL_INIT_X_P1
#define COVER_INIT_Y_P2 WELL_INIT_Y_P2
#define COVER_INIT_X_P2 WELL_INIT_X_P2

#define COVER_B_BNDRY COVER_INIT_Y + COVER_HEIGHT

#define HOLD_ID 2

#define HOLD_HEIGHT 6
#define HOLD_WIDTH 8
#define HOLD_INIT_Y WELL_INIT_Y + COVER_HEIGHT - 1
#define HOLD_INIT_X WELL_INIT_X - 10

#define HOLD_INIT_Y_P1 WELL_INIT_Y + COVER_HEIGHT - 1
#define HOLD_INIT_X_P1 WELL_INIT_X_P1 + 24

#define HOLD_INIT_Y_P2 WELL_INIT_Y + COVER_HEIGHT - 1
#define HOLD_INIT_X_P2 WELL_INIT_X_P2 - 10

#define HOLD_L_BNDRY 1
#define HOLD_R_BNDRY HOLD_WIDTH - 2
#define HOLD_T_BNDRY 1
#define HOLD_B_BNDRY HOLD_HEIGHT - 2

#define LINE_COUNT_ID 3

#define LINE_COUNT_HEIGHT 3
#define LINE_COUNT_WIDTH 15
#define LINE_COUNT_INIT_Y WELL_HEIGHT - 5
#define LINE_COUNT_INIT_X HOLD_INIT_X - 6

#define SCORE_ID 4

#define SCORE_HEIGHT 3
#define SCORE_WIDTH 10
#define SCORE_INIT_Y HOLD_INIT_Y
#define SCORE_INIT_X WELL_INIT_X + WELL_WIDTH + 5

#define LEVEL_ID 5

#define LEVEL_HEIGHT 3
#define LEVEL_WIDTH 10
#define LEVEL_INIT_Y LINE_COUNT_INIT_Y - 6
#define LEVEL_INIT_X LINE_COUNT_INIT_X

#define TITLE_SMALL_ID 6

#define TITLE_SMALL_HEIGHT 10
#define TITLE_SMALL_WIDTH 15
#define TITLE_SMALL_INIT_Y LEVEL_INIT_Y
#define TITLE_SMALL_INIT_X SCORE_INIT_X

#define CONTROLS_INIT_X 8

#define NUM_CONTROLS 7

#define MOVE_LEFT 0
#define MOVE_RIGHT 1
#define MOVE_DOWN 2
#define DROP 3
#define ROTATE_CW 4
#define ROTATE_CCW 5
#define HOLD 6

/* Decimal value of needed ASCII characters */

#define QUIT_KEY 113 // q
#define RESTART_KEY 114 // r

#define P_KEY 112
#define O_KEY 111
#define ENTER_KEY 10 

#define A_KEY 97
#define D_KEY 100
#define S_KEY 115
#define W_KEY 119
#define G_KEY 103
#define F_KEY 102
#define SPACE_KEY 32

/* The maximum number of times a tetrimino can
"adjust" itself into a valid position after an invalid rotation */
//#define ADJUST_LIMIT 3

/* Struct to represent a coordinate and its associated value */
typedef struct 
{
	int y, x;
	chtype value; 	// chtype ORs together character value, attributes, and color into a single value
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

/* Struct for use as arguments for periodic_thread and lock_in_thread*/
typedef struct
{
	WINDOW *win[NUM_WINDOWS];
	TETRIMINO *tetrimino;
	COORDINATE_PAIR (*well_contents)[WELL_CONTENTS_WIDTH];
	int controls[NUM_CONTROLS];
	int difficulty;
	int mode;
	int lock_num;
	int *current_y_checkpoint;
} THREAD_ARGS;

/* EXPERIMENTAL PURPOSES */
typedef struct 
{
	WINDOW *win[NUM_WINDOWS];
	TETRIMINO *tetrimino_1;
	TETRIMINO *tetrimino_2;
	COORDINATE_PAIR (*well_contents_1)[WELL_CONTENTS_WIDTH];
	COORDINATE_PAIR (*well_contents_2)[WELL_CONTENTS_WIDTH];
	int controls_1[NUM_CONTROLS];
	int controls_2[NUM_CONTROLS];
	int mode;
	int *current_y_checkpoint_1;
	int *current_y_checkpoint_2;

} VERSUS_THREAD_ARGS;

/* Function prototypes */
void print_help_message();
int is_input_useful(int input, int controls[NUM_CONTROLS]);

/* GUI prototypes */
void ntetris_init ();
void print_title (WINDOW *win, char *title[], int title_size);
void print_menu (WINDOW *menu_win, int highlight, char *menu_choices[], int num_menu_choices);
int get_menu_choice (char *menu_choices[], int num_menu_choices);
void draw_well (WINDOW *win, TETRIMINO *tetrimino, 
			   COORDINATE_PAIR well_contents[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH]);
void clear_well (WINDOW *win);
int update_hold (WINDOW *win, int tetrimino_id);
void update_line_count(WINDOW *line_count_win);
void update_level(WINDOW *level_win);
void update_score(WINDOW *score_win);
void print_controls();
void print_title_small(WINDOW *win);

/* Game prototypes*/
void move_tetrimino (WINDOW *win, TETRIMINO *tetrimino, int direction,
					COORDINATE_PAIR well_contents[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH]);
void adjust_bits (COORDINATE_PAIR bits[NUM_BITS], int direction);
void get_rotated_bits (COORDINATE_PAIR pivot, COORDINATE_PAIR bits_to_rotate[NUM_BITS],
					  int direction);
void rotate_tetrimino (WINDOW *win, TETRIMINO *tetrimino, int direction, 
					  COORDINATE_PAIR well_contents[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH]);
void drop_tetrimino (WINDOW *win, TETRIMINO *tetrimino, int difficulty,
					COORDINATE_PAIR well_contents[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH],
					int *current_y_checkpoint);
void init_tetrimino (TETRIMINO *tetrimino, int tetrimino_id, 
					COORDINATE_PAIR well_contents[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH],
					int *current_y_checkpoint);
void lock_tetrimino_into_well(TETRIMINO *tetrimino, 
							  COORDINATE_PAIR well_contents[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH]);
void hold_tetrimino(WINDOW *hold_win, TETRIMINO *tetrimino,
					COORDINATE_PAIR well_contents[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH],
					int *current_y_checkpoint);
int get_rand_num (int lower, int upper);
void *play_ntetris_single (void *difficulty);
void *play_ntetris_versus(void *unused);
//void *get_user_input_thread (void *arguments);
//void *get_user_input_versus_thread (void *arguments);
void *periodic_thread(void *arguments);
void *lock_in_thread(void *arguments);
int equal_coords (COORDINATE_PAIR cp_1, COORDINATE_PAIR cp_2);
int equal_bits (COORDINATE_PAIR bits_1[NUM_BITS], COORDINATE_PAIR bits_2[NUM_BITS]);
void copy_bits (COORDINATE_PAIR source_bits[NUM_BITS], COORDINATE_PAIR dest_bits[NUM_BITS]);
int get_y_checkpoint (COORDINATE_PAIR bits[NUM_BITS]);
int out_of_boundaries (WINDOW *win, COORDINATE_PAIR coords);
int valid_position (WINDOW *well_win, TETRIMINO *tetrimino, COORDINATE_PAIR new_bits[NUM_BITS], 
					COORDINATE_PAIR well_contents[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH]);
int line_complete (int row, COORDINATE_PAIR well_contents[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH]);
int line_empty (int row, COORDINATE_PAIR well_contents[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH]);
void clear_line (int row, COORDINATE_PAIR well_contents[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH]);
void update_lines(WINDOW *win, TETRIMINO *tetrimino, int difficulty,
				  COORDINATE_PAIR well_contents[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH]);

extern int GAME_DELAY;
extern int GAME_OVER_FLAG;
extern int RECENT_HOLD;
extern int CURRENTLY_HELD_TETRIMINO_ID;
extern int LINE_COUNT;
extern int SCORE;
extern int CURRENT_Y_CHECKPOINT;
extern int CURRENT_Y_CHECKPOINT_2;
#endif





