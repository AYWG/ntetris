#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include "ntetris.h"



int equal_coords (COORDINATE_PAIR cp_1, COORDINATE_PAIR cp_2)
{
	if (cp_1.x == cp_2.x && cp_1.y == cp_2.y)
		return TRUE;

	return FALSE;
}

int out_of_boundaries (WINDOW *win, COORDINATE_PAIR coords)
{
	return (coords.y < 0 || coords.y > getmaxy(win) ||
			coords.x < 0 || coords.x > getmaxx(win) - 1);
}

int valid_position (WINDOW *win, TETRIMINO *tetrimino, COORDINATE_PAIR new_coords[], int num_new_coords)
{

/* 

	For each coordinate in new_coords
	1. is it within the boundaries of the window?
	AND
	2. if it's not the same as one of the current coordinates, is the character at that new coordinate empty?


	Invalid if:

	1. out of boundaries
	OR
	2. it's not one of the current coordinates (not equal to this, not equal tothat, etc.,  and the new coordinate is occupied

*/
	int invalid = 0;
	int matching_coords;

	for (int i = 0; i < num_new_coords; i++)
	{
		matching_coords = 0;
		/* Check boundaries */
		if (out_of_boundaries(win, new_coords[i]))
		{
			invalid = 1;
			break;
		}

		for (int j = 0; j < num_new_coords; j++)
		{
			matching_coords |= equal_coords(new_coords[i], tetrimino->bits[j]);
		}

		if (!matching_coords)
		{
			if (mvwinch(win, new_coords[i].y, new_coords[i].x) & A_CHARTEXT != ' ')
			{
				invalid = 1;
				break;
			}
		}
	}
	if (invalid) return FALSE;

	return TRUE;
}

void move_tetrimino (WINDOW *win, TETRIMINO *tetrimino, int direction) 
{
	// only directions allowed are: left, right, down

	/* a piece should only be allowed to move in a specific direction (ex. left)
	if every bit can move in that direction (i.e. into an unoccupied space).
	However, consider a horizontal "I" piece. Only the leftmost piece could be
	moving into an unoccupied space (if moving left). Thus, we can't really consider
	a falling tetrimino to be occupying space. 
	To show the tetrimino on the screen, we need to print it on the well. This causes
	issues if we want to use the inch() function. We would need to somehow "ignore"
	the space "taken up" by the falling tetrimino.


	This function would need to know the current status of the main game area window
	(where the edges are, what bits have formed at the bottom)
	*/
	//TETRIMINO *local_tetr = tetrimino;

	/* need to check if valid first */
	int delta_y = 0;
	int delta_x = 0;
	int i, j;
	COORDINATE_PAIR new_coords[4];

	switch(direction)
	{
		case KEY_LEFT:
			delta_x = -1;
			break;

		case KEY_RIGHT:
			delta_x = 1;
			break;

		case KEY_DOWN:
			delta_y = 1;
			break;

		default:
			break;
	}


	for (i = 0; i < 4; i++)
	{
		new_coords[i].y = tetrimino->bits[i].y + delta_y;
		new_coords[i].x = tetrimino->bits[i].x + delta_x;
	}
	
	// Check if the new coordinates are valid, update position

	if (valid_position(win, tetrimino, new_coords, 4))
	{
		for (j = 0; i < 4; j++)
		{
			tetrimino->bits[j].y = new_coords[j].y;
			tetrimino->bits[j].x = new_coords[j].x;
		}
	} 


}

void drop_tetrimino (WINDOW *win, TETRIMINO *tetrimino)
{
	// instantly move the tetrimino to where it would go if 
	// just fell naturally
}

/* Rotates the tetrimino about its pivot coordinates
(the coordinates of one of the four circles that make up the tetrimino)

WARNING: need to handle case where user attempts to rotate piece near edge of well */

void rotate_tetrimino (WINDOW *win, TETRIMINO *tetrimino) 
{
	// need to check valid rotation

	TETRIMINO *local_tetr = tetrimino;
	COORDINATE_PAIR new_coords[4];
	int temp;
	for (int i = 0; i < 4; i++)
	{
		local_tetr->bits[i].y -= local_tetr->pivot.y;
		local_tetr->bits[i].x -= local_tetr->pivot.x;

		temp = local_tetr->bits[i].y; 
		local_tetr->bits[i].y =  !local_tetr->bits[i].x + 1;
		local_tetr->bits[i].x = temp;

		local_tetr->bits[i].y += local_tetr->pivot.y;
		local_tetr->bits[i].x += local_tetr->pivot.x;
		
		new_coords[i].y = local_tetr->bits[i].y;
		new_coords[i].x = local_tetr->bits[i].x;
	}

	if (valid_position(win, tetrimino, new_coords, 4))
		tetrimino = local_tetr;
}



void init_tetrimino (TETRIMINO *tetrimino, int tetrimino_id)
{
	/*
	switch(tetrimino_id)
	{
		TETRIMINO_I: 
			{
				
			}
	}
	*/
}


/* Generate a random tetrimino ID ranging from 0 to 6 inclusive 
(one for each of the pieces) */

int get_rand_tetrimino ()
{
	int lower = 0;
	int upper = 6;

	return rand() % (upper - lower + 1) + lower;
}

/* Top-level function for running the game. */

void play_ntetris () 
{
	WINDOW *well_win;
	WINDOW *hold_win;
	WINDOW *line_count_win;
	WINDOW *score_win;

	int well_y = 1;
	int well_x = COLS / 3;

	int hold_y = 1;
	int hold_x = COLS / 5;

	int line_count_y = LINES - 4;
	int line_count_x = COLS / 5;

	int score_y = LINES - 4;
	int score_x = 4 * COLS / 5;

	well_win = newwin(LINES - 2, well_x, well_y, well_x);
	hold_win = newwin(6, 6, hold_y, hold_x);
	line_count_win = newwin(2, 4, line_count_y, line_count_x);
	score_win = newwin(2, 4, score_y, score_x);


	box(well_win, 0, 0);
	box(hold_win, 0, 0);
	box(line_count_win, 0, 0);
	box(score_win, 0, 0);
	wrefresh(well_win);
	wrefresh(hold_win);
	wrefresh(line_count_win);
	wrefresh(score_win);

	/* Generate random number seed*/
	/*
	srand((unsigned) time(NULL));

	while ((ch = getch()) != QUIT_KEY)
	{

	}
	*/
}