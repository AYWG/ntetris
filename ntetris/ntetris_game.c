#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include "ntetris.h"

void move_tetrimino (TETRIMINO *tetrimino, int direction) 
{
	// only directions allowed are: left, right, down

	/* a piece should only be allowed to move in a specific direction (ex. left)
	if every bit can move in that direction (i.e. into an unoccupied space).
	However, consider a horizontal "I" piece. Only the leftmost piece could be
	moving into an unoccupied space (if moving left). Thus, we can't really consider
	a falling tetrimino to be occupying space. 
	To show the tetrimino on the screen, we need to print it on the well. This causes
	issues if we want to use the inch() function. We would need to somehow "ignore"
	the space "taken up" by the falling tetrimino

	if (space is unoccupied or space is occupied by tetrimino bit)  

	This function would need to know the current status of the main game area window
	(where the edges are, what bits have formed at the bottom)
	*/


}

void drop_tetrimino (TETRIMINO *tetrimino)
{
	// instantly move the tetrimino to where it would go if 
	// just fell naturally
}

/* Rotates the tetrimino about its pivot coordinates
(the coordinates of one of the four circles that make up the tetrimino)

WARNING: need to handle case where user attempts to rotate piece near edge of well */

void rotate_tetrimino (TETRIMINO *tetrimino) 
{
	int temp;
	for (int i = 0; i < 4; i++)
	{
		tetrimino->bits[i].y -= tetrimino->pivot.y;
		tetrimino->bits[i].x -= tetrimino->pivot.x;

		temp = tetrimino->bits[i].y; 
		tetrimino->bits[i].y =  !tetrimino->bits[i].x + 1;
		tetrimino->bits[i].x = temp;

		tetrimino->bits[i].y += tetrimino->pivot.y;
		tetrimino->bits[i].x += tetrimino->pivot.x;
	}
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