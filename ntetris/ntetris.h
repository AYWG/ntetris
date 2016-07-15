#ifndef NTETRIS_H
#define NTETRIS_H

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

/* IDs of the different game pieces */
#define TETRIMINO_I 0
#define TETRIMINO_J 1
#define TETRIMINO_L 2
#define TETRIMINO_O 3 
#define TETRIMINO_S 4
#define TETRIMINO_T 5
#define TETRIMINO_Z 6


typedef struct 
{
	int y, x;
} COORDINATE_PAIR;

typedef struct 
{
	int tetrimino_type;

	COORDINATE_PAIR pivot;

	/* The coordinates of each of
	the 4 bits that make up a tetrimino*/
	COORDINATE_PAIR bits[4];

} TETRIMINO;


void ntetris_init ();
void print_title ();
void print_menu (WINDOW *menu_win, int highlight);
int get_menu_choice ();


void move_tetrimino (WINDOW *win, TETRIMINO *tetrimino, int direction);
void rotate_tetrimino (WINDOW *win, TETRIMINO *tetrimino);
void drop_tetrimino (WINDOW *win, TETRIMINO *tetrimino);
void init_tetrimino (TETRIMINO *tetrimino, int tetrimino_id);
int get_rand_tetrimino ();
void play_ntetris ();
int check_equal_coords (COORDINATE_PAIR cp_1, COORDINATE_PAIR cp_2);
int out_of_boundaries (WINDOW *win, COORDINATE_PAIR coords);
int valid_position (WINDOW *win, TETRIMINO *tetrimino, COORDINATE_PAIR new_coords[], int num_new_coords);

#endif





