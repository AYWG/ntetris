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
#define HOWTOPLAY 3
#define EXIT 4

/* Maximum number of players */
typedef enum {
	PLAYER_1,
	PLAYER_2,
	NUM_PLAYERS
} EPlayer;

typedef enum {
	NOT_OVER,
	PLAYER_1_LOST,
	PLAYER_2_LOST
} EGameOver;

/* Difficulty levels */
typedef enum {
	CASUAL,
	INTERMEDIATE,
	EXPERT,
	INVALID_DIFFICULTY
} EDifficulty;

/* Game delays (in microseconds) */
#define CASUAL_INIT_DELAY 1000000
#define INTERMEDIATE_INIT_DELAY 500000
#define EXPERT_INIT_DELAY 250000
#define MIN_DELAY 30000 // Game delay cannot go below this amount
#define STALL 1000

/* IDs of the different game pieces */
typedef enum {
	TETRIMINO_I, 
	TETRIMINO_J, 
	TETRIMINO_L, 
	TETRIMINO_O, 
	TETRIMINO_S, 
	TETRIMINO_T, 
	TETRIMINO_Z, 
	NUM_TETRIMINOS, 
	INVALID_ID
} ETetrimino;

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
typedef enum {
	UP,
	DOWN,
	LEFT,
	RIGHT,
	CW,
	CCW
} EDirection;

/* Dimensions and initial coordinates for 
the WINDOWs used */
typedef enum {
	WELL_ID,
	COVER_ID,
	HOLD_ID,
	LINE_COUNT_ID,
	SCORE_ID,
	LEVEL_ID,
	TITLE_SMALL_ID,
	GARBAGE_ID,
	OTHER_GARBAGE_ID,
	NUM_WINDOWS
} EWindow;
#define NUM_WINDOWS 9

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

#define COVER_HEIGHT 3
#define COVER_WIDTH WELL_WIDTH
#define COVER_INIT_Y WELL_INIT_Y
#define COVER_INIT_X WELL_INIT_X

#define COVER_INIT_Y_P1 WELL_INIT_Y_P1
#define COVER_INIT_X_P1 WELL_INIT_X_P1
#define COVER_INIT_Y_P2 WELL_INIT_Y_P2
#define COVER_INIT_X_P2 WELL_INIT_X_P2

#define COVER_B_BNDRY COVER_INIT_Y + COVER_HEIGHT

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

#define LINE_COUNT_HEIGHT 3
#define LINE_COUNT_WIDTH 15
#define LINE_COUNT_INIT_Y WELL_HEIGHT - 5
#define LINE_COUNT_INIT_X HOLD_INIT_X - 6

#define SCORE_HEIGHT 3
#define SCORE_WIDTH 10
#define SCORE_INIT_Y HOLD_INIT_Y
#define SCORE_INIT_X WELL_INIT_X + WELL_WIDTH + 5

#define LEVEL_HEIGHT 3
#define LEVEL_WIDTH 10
#define LEVEL_INIT_Y LINE_COUNT_INIT_Y - 6
#define LEVEL_INIT_X LINE_COUNT_INIT_X

#define TITLE_SMALL_HEIGHT 10
#define TITLE_SMALL_WIDTH 15
#define TITLE_SMALL_INIT_SINGLE_Y LEVEL_INIT_Y
#define TITLE_SMALL_INIT_SINGLE_X SCORE_INIT_X
#define TITLE_SMALL_INIT_VERSUS_Y 10
#define TITLE_SMALL_INIT_VERSUS_X 40

#define GARBAGE_HEIGHT 5
#define GARBAGE_WIDTH 9
#define GARBAGE_INIT_Y_P1 WELL_HEIGHT - 6
#define GARBAGE_INIT_X_P1 HOLD_INIT_X_P1
#define GARBAGE_INIT_Y_P2 WELL_HEIGHT - 6
#define GARBAGE_INIT_X_P2 HOLD_INIT_X_P2

/* For the menu option */

#define CONTROLS_INIT_X 8
#define HOWTOPLAY_INIT_X 6

/* Constants representing the different controls */
typedef enum {
	MOVE_LEFT,
	MOVE_RIGHT,
	MOVE_DOWN,
	DROP,
	ROTATE_CW,
	ROTATE_CCW,
	HOLD,
	NUM_CONTROLS
} EControls;

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
	ETetrimino tetrimino_type;

	/* The bit that is used as pivot when the tetrimino rotates */
	int pivot_bit;

	/* The coordinates of each of
	the bits that make up the tetrimino*/
	COORDINATE_PAIR bits[NUM_BITS];

	/* Lock for modifying the tetrimino*/
	pthread_mutex_t lock;
} TETRIMINO;

typedef struct
{
	int counter;
	pthread_mutex_t lock;
} GarbageLine;

typedef struct
{
	TETRIMINO tetrimino[NUM_PLAYERS];
	ETetrimino currently_held_tetrimino[NUM_PLAYERS];
	EDifficulty difficulty;
	COORDINATE_PAIR well_contents[NUM_PLAYERS][WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH];
	EGameOver game_over_flag;	
	GarbageLine garbage_line[NUM_PLAYERS];
	int has_held_recently[NUM_PLAYERS];
	int mode;
	int game_delay;
	int line_count;
	int score; 
	int current_y_checkpoint[NUM_PLAYERS];
	int well_max_y[NUM_PLAYERS];
	int well_max_x[NUM_PLAYERS];
} GameState;

typedef struct
{
	int refresh_delay;
	GameState *state;
	WINDOW *win[NUM_PLAYERS][NUM_WINDOWS];
} GUI;

/* Struct for use as arguments for periodic_thread and lock_in_thread*/
typedef struct
{
	GameState *state;
	EPlayer player_id;
} ThreadArgs;

/* Struct for server_send_thread and client_recv_thread*/
typedef struct
{
	int client_server_socket;
	GameState *state;

} ClientServerThreadArgs;

/* Struct for game data that server sends to client */
typedef struct
{
	EGameOver game_over_flag;
	ETetrimino currently_held_tetrimino[NUM_PLAYERS];
	COORDINATE_PAIR tetrimino_bits[NUM_PLAYERS][NUM_BITS];
	COORDINATE_PAIR well_contents[NUM_PLAYERS][WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH];
	int garbage_line_counter[NUM_PLAYERS];
} ServerResponse;

/* Main prototypes */
void print_help_message();
void print_howtoplay_message();

/* Thread prototypes */
int is_input_useful(int input, int controls[NUM_CONTROLS]);
void add_garbage(GameState *state, EPlayer from_player, EPlayer to_player, int num_complete_lines);
void *periodic_thread(void *arguments);
void *lock_in_thread(void *arguments);
void *run_gui(void *ui);
void *server_send_thread(void *send_args);
void *client_recv_thread(void *recv_args);

/* GUI prototypes */
void ntetris_init ();
void gui_init(GUI *gui, GameState *state);
void gui_cleanup(GUI *gui, int mode);
void print_title (WINDOW *win, char *title[], int title_size);
void print_menu (WINDOW *menu_win, int highlight, char *menu_choices[], int num_menu_choices);
int get_menu_choice (char *menu_choices[], int num_menu_choices);
void update_well(GUI *gui, EPlayer player_id, COORDINATE_PAIR tetrimino_bits[NUM_BITS], COORDINATE_PAIR well_contents[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH]);
void update_hold(GUI *gui, EPlayer player_id, int tetrimino_id);
void update_line_count(GUI *gui, EPlayer player_id);
void update_level(GUI *gui, EPlayer player_id);
void update_score(GUI *gui, EPlayer player_id);
void update_garbage_line_counter(GUI *gui, EPlayer player_id, int garbage_counter);
void print_controls();
void print_howtoplay();
void print_title_small(GUI *gui);

/* Game prototypes*/
void game_state_init(GameState *state, EDifficulty difficulty, int mode);
void play_ntetris_single (GameState *state);
void play_ntetris_versus (GameState *state);
void copy_bits (COORDINATE_PAIR source_bits[NUM_BITS], COORDINATE_PAIR dest_bits[NUM_BITS]);
int get_y_checkpoint (COORDINATE_PAIR bits[NUM_BITS]);
int valid_position (int well_max_x, int well_max_y, COORDINATE_PAIR new_bits[NUM_BITS], COORDINATE_PAIR well_contents[WELL_HEIGHT][WELL_CONTENTS_WIDTH]);
void move_tetrimino (GameState *state, EPlayer player_id, EDirection direction);
void get_rotated_bits (COORDINATE_PAIR pivot, COORDINATE_PAIR bits_to_rotate[NUM_BITS],
					  EDirection direction);
void rotate_tetrimino (GameState *state, EPlayer player_id, EDirection direction);
int drop_tetrimino (GameState *state, EPlayer player_id);
void init_tetrimino (GameState *state, EPlayer player_id, ETetrimino tetrimino_id);
void lock_tetrimino_into_well(GameState *state, EPlayer player_id);
void hold_tetrimino(GameState *state, EPlayer player_id);
int get_rand_num (int lower, int upper);
int line_empty (int row, COORDINATE_PAIR well_contents[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH]);
int update_lines(GameState *state, EPlayer player_id);

/* Networking stuff */
void play_ntetris_remote(GameState *local_game_state);
int connect_to_server(const char * hostname);

#endif






