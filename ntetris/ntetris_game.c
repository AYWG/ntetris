/* Gameplay-related functions for ntetris */

#include "ntetris.h"

int base_pts_for_line_clears[] = {40, 100, 300, 1200};


/* Determines whether cp_1 is located at the same coordinates to cp_2. */

int equal_coords (COORDINATE_PAIR cp_1, COORDINATE_PAIR cp_2)
{
	return (cp_1.x == cp_2.x) && (cp_1.y == cp_2.y);
}

int equal_bits (COORDINATE_PAIR bits_1[], COORDINATE_PAIR bits_2[], int num_bits)
{
	int i;
	int equal = 1;

	for (i = 0; i < num_bits; i++)
	{
		equal &= equal_coords(bits_1[i], bits_2[i]);
	}

	return equal;
}

/* Copies the location (not value) of each COORDINATE_PAIR in source_bits into dest_bits*/

void copy_bits (COORDINATE_PAIR source_bits[], COORDINATE_PAIR dest_bits[], int num_bits)
{
	int i;
	for (i = 0; i < num_bits; i++)
	{
		dest_bits[i].y = source_bits[i].y;
		dest_bits[i].x = source_bits[i].x;
	}
}

/* Determines whether the given coords are outside the boundaries
of win.
 */

/* Probably don't need the window parameter, should remove it */

int out_of_boundaries (WINDOW *win, COORDINATE_PAIR coords)
{
	return (coords.y < WELL_T_BNDRY || coords.y > getmaxy(win) - 2 ||
			coords.x < WELL_L_BNDRY || coords.x > getmaxx(win) - 2);
}

/* Determines if new_bits is a valid array of bits for the tetrimino within
the window. */

int valid_position (WINDOW *win, TETRIMINO *tetrimino, COORDINATE_PAIR new_bits[], int num_bits)
{
	int invalid = 0;
	int row, col;
	int i;

	for (i = 0; i < num_bits; i++)
	{
		/* Check boundaries */
		if (out_of_boundaries(win, new_bits[i]))
		{
			invalid = 1;
			break;
		}

		row = new_bits[i].y - 1;
		col = new_bits[i].x - 1;

		if ((well_contents[row][col].value & A_CHARTEXT != ' ') &&
			(well_contents[row][col].value & A_ATTRIBUTES) != A_DIM)
		{
			invalid = 1;
			break;
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
	int i;
	COORDINATE_PAIR new_bits[NUM_BITS];

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
	}

	for (i = 0; i < NUM_BITS; i++)
	{
		new_bits[i].y = tetrimino->bits[i].y + delta_y;
		new_bits[i].x = tetrimino->bits[i].x + delta_x;
	}
	
	/* if the new coordinates are valid, update position */

	if (valid_position(win, tetrimino, new_bits, NUM_BITS))
	{
		copy_bits(new_bits, tetrimino->bits, NUM_BITS);
		if (direction == KEY_DOWN)
			SCORE++;
	}

}

/* Instantly move the tetrimino to where it would go if 
   just fell naturally down the well from its current position,
   then lock it into the well. 
 */

void drop_tetrimino (WINDOW *win, TETRIMINO *tetrimino, int difficulty)
{
	COORDINATE_PAIR new_bits[NUM_BITS];
	int i;
	int distance_traveled = 0;
	copy_bits(tetrimino->bits, new_bits, NUM_BITS);

	while (valid_position(win, tetrimino, new_bits, NUM_BITS))
	{
		for (i = 0; i < NUM_BITS; i++)
			new_bits[i].y++;
		distance_traveled++;
	}
	for (i = 0; i < NUM_BITS; i++)
			new_bits[i].y--;
	distance_traveled--;

	copy_bits(new_bits, tetrimino->bits, NUM_BITS);

	lock_tetrimino_into_well(tetrimino);
	update_lines(win, tetrimino, difficulty);
	SCORE += 2 * distance_traveled;
	init_tetrimino(win, tetrimino, get_rand_num(0, 6));
	draw_well(win, tetrimino);
}

void adjust_bits (COORDINATE_PAIR bits[], int num_bits, int direction)
{
	int delta_y, delta_x;
	int i;

	switch(direction)
	{
		case UP: delta_y = -1; delta_x = 0; break;
		case DOWN: delta_y = 1; delta_x = 0; break;
		case LEFT: delta_y = 0; delta_x = -1; break;
		case RIGHT: delta_y = 0; delta_x = 1; break;
	}

	for (i = 0; i < num_bits; i++)
	{
		bits[i].y += delta_y;
		bits[i].x += delta_x;
	}
}

void get_rotated_bits (COORDINATE_PAIR pivot, COORDINATE_PAIR old_bits[], COORDINATE_PAIR new_bits[], int num_bits, int direction)
{
	int temp, i;
	for (i = 0; i < num_bits; i++)
	{
		old_bits[i].y -= pivot.y;
		old_bits[i].x -= pivot.x;

		if (direction == CLOCKWISE)
		{
			temp = old_bits[i].y; 
			old_bits[i].y =  -old_bits[i].x;
			old_bits[i].x = temp;
		}
		else // CNT_CLOCKWISE
		{
			temp = old_bits[i].x; 
			old_bits[i].x =  -old_bits[i].y;
			old_bits[i].y = temp;
		}

		old_bits[i].y += pivot.y;
		old_bits[i].x += pivot.x;
		
		new_bits[i].y = old_bits[i].y;
		new_bits[i].x = old_bits[i].x;
	}
}

/*
 Rotates the tetrimino about its pivot coordinates
(the coordinates of one of the four o's that make up the tetrimino)
*/

void rotate_tetrimino (WINDOW *win, TETRIMINO *tetrimino, int direction) 
{

	/* Only rotate if the tetrimino is not an O piece */
	if (tetrimino->tetrimino_type != TETRIMINO_O)
	{
		COORDINATE_PAIR old_bits[NUM_BITS];
		COORDINATE_PAIR new_bits[NUM_BITS];
		COORDINATE_PAIR pivot;

		int i;
		int coords_out_of_y_bounds, coords_out_of_x_bounds;
		int delta_y, delta_x;
		int count_adjust = 0;

		pivot.y = tetrimino->bits[tetrimino->pivot_bit].y;
		pivot.x = tetrimino->bits[tetrimino->pivot_bit].x;

		copy_bits(tetrimino->bits, old_bits, NUM_BITS);
		get_rotated_bits(pivot, old_bits, new_bits, NUM_BITS, direction);

		while (!valid_position(win, tetrimino, new_bits, NUM_BITS))
		{
			/* Check if at least one of the new bits is out of bounds*/
			coords_out_of_y_bounds = 0;
			coords_out_of_x_bounds = 0;

			for (i = 0; i < NUM_BITS; i++)
			{
				if (new_bits[i].y < WELL_T_BNDRY || new_bits[i].y > WELL_B_BNDRY)
					coords_out_of_y_bounds++;

				if (new_bits[i].x < WELL_L_BNDRY || new_bits[i].x > WELL_R_BNDRY)
					coords_out_of_x_bounds++;
			}

			if (coords_out_of_y_bounds)
			{
				/* Closer to the top? */
				if (abs(tetrimino->bits[0].y - WELL_T_BNDRY) < abs(tetrimino->bits[0].y - WELL_B_BNDRY))
					adjust_bits(new_bits, NUM_BITS, DOWN);
				else 
					adjust_bits(new_bits, NUM_BITS, UP);
			}

			if (coords_out_of_x_bounds)
			{
				/* Closer to the left? */
				if (abs(tetrimino->bits[0].x - WELL_L_BNDRY) < abs(tetrimino->bits[0].x - WELL_R_BNDRY))
					adjust_bits(new_bits, NUM_BITS, RIGHT);
				else 
					adjust_bits(new_bits, NUM_BITS, LEFT);
			}

			if (coords_out_of_y_bounds == 0 && coords_out_of_x_bounds == 0)
			{
				if (count_adjust > ADJUST_LIMIT)
					return;

				adjust_bits(new_bits, NUM_BITS, get_rand_num(1, 4));
				count_adjust++;
			}

		}

		copy_bits(new_bits, tetrimino->bits, NUM_BITS);
	
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

	/* don't actually need the win parameter */

	int a, b, c, d;
	int e, f, g, h;

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
		if ((well_contents[init_y[i] - 1][init_x[i] - 1].value & A_CHARTEXT) != ' ')
		{
			GAME_OVER_FLAG = 1;
			return;
		}

		tetrimino->bits[i].y = init_y[i];
		tetrimino->bits[i].x = init_x[i];

		/* Offset of 3 between ID number and COLOUR_PAIR number*/
		tetrimino->bits[i].value = 'o' | COLOR_PAIR(tetrimino_id + 3);
	}
}

void lock_tetrimino_into_well(TETRIMINO *tetrimino)
{
	int row, col;
	int i;
	for (i = 0; i < NUM_BITS; i++)
	{
		row = tetrimino->bits[i].y - 1;
		col = tetrimino->bits[i].x - 1;
		well_contents[row][col].value = tetrimino->bits[i].value;
	}
	RECENT_HOLD = 0;
}

void hold_tetrimino(WINDOW *well_win, WINDOW *hold_win, TETRIMINO *tetrimino)
{
	if (!RECENT_HOLD)
	{
		int old_id = update_hold(hold_win, tetrimino->tetrimino_type);

		if (old_id != INVALID_ID)
			init_tetrimino(well_win, tetrimino, old_id);
		else
			init_tetrimino(well_win, tetrimino, get_rand_num(0, 6));

		RECENT_HOLD = 1;
	}
}


/* Generate a random number ranging from lower to upper inclusive */

int get_rand_num (int lower, int upper)
{
	return rand() % (upper - lower + 1) + lower;
}

/* Checks if a line in the window is "complete" - that is,
all coordinates are occupied by an 'o' that is not one of the current
tetrimino bits */

int line_complete (int row)
{
	int complete = 1;
	int j;
	for (j = 0; j < WELL_R_BNDRY; j++)
	{
		complete &= ((well_contents[row][j].value & A_CHARTEXT) == 'o');
	}

	return complete;
}

int line_empty (int row)
{
	int empty = 1;
	int j;
	for (j = 0; j < WELL_R_BNDRY; j++)
	{
		empty &= (well_contents[row][j].value == ' ');
	}

	return empty;
}

void clear_line (int row)
{
	int j;

	for (j = 0; j < WELL_R_BNDRY; j++)
	{
		well_contents[row][j].value = ' ';	
	}
}

void update_lines(WINDOW *win, TETRIMINO *tetrimino, int difficulty)
{
	
	int num_complete_lines = 0;
	int i, j;
	int complete = 0;
	int consec_complete_lines = 0;

	for (i = WELL_B_BNDRY - 1; i >= 0; i--)
	{
		if (line_empty(i)) break;

		if (line_complete(i))
		{
			for (j = 0; j < WELL_R_BNDRY; j++)
				well_contents[i][j].value |= A_REVERSE;
			complete = 1;
			consec_complete_lines++;
		}
		else
		{
			if (consec_complete_lines > 0)
				SCORE += (base_pts_for_line_clears[consec_complete_lines - 1] * ((LINE_COUNT / 10) + 1 + difficulty));
			
			consec_complete_lines = 0;
		} 
	}

	draw_well(win, tetrimino);
	if (complete)
		usleep(GAME_DELAY);

	for (i = WELL_B_BNDRY - 1; i >= 0; i--)
	{
		if (line_empty(i)) break;

		if (line_complete(i))
		{
			clear_line(i);
			num_complete_lines++;
			LINE_COUNT++;
			if (LINE_COUNT % 10 == 0) 
			{
				switch(difficulty)
				{
					case CASUAL:
						GAME_DELAY -= CASUAL_INIT_DELAY / 20; break;
					case INTERMEDIATE:
						GAME_DELAY -= INTERMEDIATE_INIT_DELAY / 20; break;
					case EXPERT:
						GAME_DELAY -= EXPERT_INIT_DELAY / 20; break;
				}
			}
		}
		else
		{
			// copy line i to line i + num_complete_lines
			if (i + num_complete_lines != i)
			{
				for (j = 0; j < WELL_R_BNDRY; j++)
				{
					well_contents[i + num_complete_lines][j].value = well_contents[i][j].value;
				}
				clear_line(i);
			}
		}

	}
	
}