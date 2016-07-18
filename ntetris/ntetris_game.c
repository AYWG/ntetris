#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include "ntetris.h"

/* Determines whether cp_1 is equal to cp_2.
Returns 1 if true, 0 if false. */

int equal_coords (COORDINATE_PAIR cp_1, COORDINATE_PAIR cp_2)
{
	if (cp_1.x == cp_2.x && cp_1.y == cp_2.y)
		return TRUE;

	return FALSE;
}

/* Determines whether the given coords are outside the boundaries
of win.
Returns 1 if true, 0 if false. */

int out_of_boundaries (WINDOW *win, COORDINATE_PAIR coords)
{
	return (coords.y < 1 || coords.y > getmaxy(win) - 1||
			coords.x < 1 || coords.x > getmaxx(win) - 1);
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

/* Updates the coordinates of */

void move_tetrimino (WINDOW *win, TETRIMINO *tetrimino, int direction) 
{
	int delta_y = 0;
	int delta_x = 0;
	int i, j;
	COORDINATE_PAIR new_coords[NUM_BITS];

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


	for (i = 0; i < NUM_BITS; i++)
	{
		new_coords[i].y = tetrimino->bits[i].y + delta_y;
		new_coords[i].x = tetrimino->bits[i].x + delta_x;
	}
	
	// Check if the new coordinates are valid, update position

	if (valid_position(win, tetrimino, new_coords, 4))
	{
		for (j = 0; i < NUM_BITS; j++)
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

	/* First, get its current coordinates. Then, 
	keep decreasing y coordinate of each bit by 1 and
	check for valid_position - when this returns 0, stop, 
	then update tetrimino's coordinates
	*/


}

/* Makes a copy of the given tetrimino. */

TETRIMINO *copy_tetrimino (TETRIMINO *tetrimino)
{
	TETRIMINO *copied_tetr = malloc(sizeof(TETRIMINO));

	copied_tetr->tetrimino_type = tetrimino->tetrimino_type;
	copied_tetr->pivot_bit = tetrimino->pivot_bit;
	for (int i = 0; i < NUM_BITS; i++)
	{
		copied_tetr->bits[i].y = tetrimino->bits[i].y;
		copied_tetr->bits[i].x = tetrimino->bits[i].x;
	}

	return copied_tetr;
}

/* Rotates the tetrimino about its pivot coordinates
(the coordinates of one of the four circles that make up the tetrimino)

WARNING: need to handle case where user attempts to rotate piece near edge of well */

void rotate_tetrimino (WINDOW *win, TETRIMINO *tetrimino) 
{
	// need to check valid rotation

	TETRIMINO *local_tetr = copy_tetrimino(tetrimino);
	COORDINATE_PAIR new_coords[NUM_BITS];
	COORDINATE_PAIR pivot;

	pivot.y = local_tetr->bits[local_tetr->pivot_bit].y;
	pivot.x = local_tetr->bits[local_tetr->pivot_bit].x;

	int temp;
	for (int i = 0; i < NUM_BITS; i++)
	{
		local_tetr->bits[i].y -= pivot.y;
		local_tetr->bits[i].x -= pivot.x;

		temp = local_tetr->bits[i].y; 
		local_tetr->bits[i].y =  !local_tetr->bits[i].x + 1;
		local_tetr->bits[i].x = temp;

		local_tetr->bits[i].y += pivot.y;
		local_tetr->bits[i].x += pivot.x;
		
		new_coords[i].y = local_tetr->bits[i].y;
		new_coords[i].x = local_tetr->bits[i].x;
	}

	if (valid_position(win, tetrimino, new_coords, 4))
	{
		free(tetrimino);
		tetrimino = local_tetr;
	}
	else
	{
		free(local_tetr);
	}
}

/*
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


void init_tetrimino (WINDOW *win, TETRIMINO *tetrimino, int tetrimino_id)
{
	/* Need to check if tetrimino can actually be initialized (if there's space);
	If not, then game over */

	int a, b, c, d;
	int e, f, g, h;

	COORDINATE_PAIR init_coords[NUM_BITS]; 

	switch(tetrimino_id)
	{
		case TETRIMINO_I: 
			tetrimino->tetrimino_type = TETRIMINO_I;
			tetrimino->pivot_bit = 1;
			a = b = c = d = 1;
			e = 11; f = e + 1; g = f + 1; h = g + 1;
			break;
			
		case TETRIMINO_J:
			tetrimino->tetrimino_type = TETRIMINO_J;
			tetrimino->pivot_bit = 1;
			a = b = c = 1; d = 2;
			e = 11; f = e + 1; g = f + 1; h = g;
			break;

		case TETRIMINO_L:
			tetrimino->tetrimino_type = TETRIMINO_L;
			tetrimino->pivot_bit = 1;
			a = b = c = 1; d = 2;
			e = 11; f = e + 1; g = f + 1; h = e;
			break;

		case TETRIMINO_O:
			tetrimino->tetrimino_type = TETRIMINO_O;	
			tetrimino->pivot_bit = 0; // actually has no pivot bit
			a = b = 1; c = d = 2;
			e = g = 12; f = h = 13;
			break;

		case TETRIMINO_S:
			tetrimino->tetrimino_type = TETRIMINO_S;
			tetrimino->pivot_bit = 3;
			a = b = 1; c = d = 2;
			e = h = 12; f = 13; g = 11;
			break;

		case TETRIMINO_T:
			tetrimino->tetrimino_type = TETRIMINO_T;
			tetrimino->pivot_bit = 1;
			a = b = c = 1; d = 2;
			e = 11; f = e + 1; g = f + 1; h = f;
			break;

		case TETRIMINO_Z:
			tetrimino->tetrimino_type = TETRIMINO_Z;
			tetrimino->pivot_bit = 2;
			a = b = 1; c = d = 2;
			e = 11; f = g = 12; h = 13;
			break;
	}

	int init_y[NUM_BITS] = {a, b, c, d};
	int init_x[NUM_BITS] = {e, f, g, h};


	for (int i = 0; i < NUM_BITS; i++)
	{
		tetrimino->bits[i].y = init_y[i];
		tetrimino->bits[i].x = init_x[i];
	}
	
}


/* Generate a random tetrimino ID ranging from 0 to 6 inclusive 
(one for each of the pieces) */

int get_rand_tetrimino ()
{
	int lower = 0;
	int upper = 6;

	return rand() % (upper - lower + 1) + lower;
}

