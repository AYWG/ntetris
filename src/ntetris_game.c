/* Gameplay-related functions for ntetris */

#include "ntetris.h"

int base_pts_for_line_clears[] = {40, 100, 300, 1200};

int init_y[NUM_TETRIMINOS][NUM_BITS] = {{1, 1, 1, 1},
										{1, 1, 1, 2},
										{1, 1, 1, 2},
										{1, 1, 2, 2},
										{1, 1, 2, 2},
										{1, 1, 1, 2},
										{1, 1, 2, 2}
										};

int init_x[NUM_TETRIMINOS][NUM_BITS] = {{11, 12, 13, 14},
										{11, 12, 13, 13},
										{11, 12, 13, 11},
										{12, 13, 12, 13},
										{12, 13, 11, 12},
										{11, 12, 13, 12},
										{11, 12, 12, 13} 
										};

int pivot_bits[NUM_TETRIMINOS] = {1, 1, 1, 0, 3, 1, 2};										


/* Determines whether cp_1 is located at the same coordinates as cp_2. */

int equal_coords (COORDINATE_PAIR cp_1, COORDINATE_PAIR cp_2)
{
	return (cp_1.x == cp_2.x) && (cp_1.y == cp_2.y);
}

/* Determines whether each bit in bits_1 has the same coordinates as the corresponding bit
in bits_2 */

int equal_bits (COORDINATE_PAIR bits_1[NUM_BITS], COORDINATE_PAIR bits_2[NUM_BITS])
{
	int i;
	for (i = 0; i < NUM_BITS; i++)
		if (!equal_coords(bits_1[i], bits_2[i]))
			return FALSE;
	
	return TRUE;
}

/* Copies the location of each bit in source_bits into dest_bits */

void copy_bits (COORDINATE_PAIR source_bits[NUM_BITS], COORDINATE_PAIR dest_bits[NUM_BITS])
{
	int i;

	for (i = 0; i < NUM_BITS; i++)
	{
		dest_bits[i].y = source_bits[i].y;
		dest_bits[i].x = source_bits[i].x;
	}
}

/* Returns the y checkpoint for the given bits. The y checkpoint is defined as the 
largest y coordinate out of all of bits' y coordinates, plus one. */

int get_y_checkpoint (COORDINATE_PAIR bits[NUM_BITS])
{
	int highest_y_coord = 0;
	int i;

	for (i = 0; i < NUM_BITS; i++)
	{
		if (bits[i].y > highest_y_coord)
			highest_y_coord = bits[i].y;
	}

	return highest_y_coord + 1;
}

/* Determines whether the given coords are outside the boundaries
of win (assuming that win has a border) */

int out_of_boundaries (WINDOW *win, COORDINATE_PAIR coords)
{
	return (coords.y < 1 || coords.y > getmaxy(win) - 2 ||
			coords.x < 1 || coords.x > getmaxx(win) - 2);
}

/* Determines if new_bits is a valid array of bits for the tetrimino within
the well window. */

int valid_position (WINDOW *well_win, TETRIMINO *tetrimino, COORDINATE_PAIR new_bits[NUM_BITS], 
					COORDINATE_PAIR well_contents[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH])
{
	int row, col;
	int i;

	for (i = 0; i < NUM_BITS; i++)
	{
		/* Check boundaries */
		if (out_of_boundaries(well_win, new_bits[i]))
			return FALSE;
		
		row = new_bits[i].y - 1;
		col = new_bits[i].x - 1;

		/* Valid coordinates in the well are those unoccupied or occupied by the 
		tetrimino's "ghost" */
		if ((well_contents[row][col].value & A_CHARTEXT != ' ') &&
			(well_contents[row][col].value & A_ATTRIBUTES) != A_DIM)
			return FALSE;
		
	}
	return TRUE;
}

/* Updates the tetrimino's location based on the given direction, if possible. 
Allowed directions include: left, right, and down. If the new location is invalid
(e.g. something is in the way) then nothing happens */

void move_tetrimino (WINDOW *win, TETRIMINO *tetrimino, int direction,
					COORDINATE_PAIR well_contents[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH]) 
{
	int delta_y = 0;
	int delta_x = 0;
	int i;
	COORDINATE_PAIR new_bits[NUM_BITS];

	switch(direction)
	{
		case LEFT: delta_x = -1; break;
		case RIGHT: delta_x = 1; break;
		case DOWN: delta_y = 1; break;
	}

	for (i = 0; i < NUM_BITS; i++)
	{
		new_bits[i].y = tetrimino->bits[i].y + delta_y;
		new_bits[i].x = tetrimino->bits[i].x + delta_x;
	}
	
	/* if the new coordinates are valid, update position */

	if (valid_position(win, tetrimino, new_bits, well_contents))
	{
		copy_bits(new_bits, tetrimino->bits);
		/* "soft" drops award 1 point */
		if (direction == DOWN) 
			SCORE++;
	}
}

/* Instantly move the tetrimino to where it would go if 
just fell naturally down the well from its current position,
then lock it into the well. Returns the number of lines cleared
as a result of dropping the tetrimino */

int drop_tetrimino (WINDOW *win, TETRIMINO *tetrimino, int difficulty,
					COORDINATE_PAIR well_contents[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH],
					int *current_y_checkpoint, int *recent_hold)
{
	COORDINATE_PAIR new_bits[NUM_BITS];
	int i;
	int num_complete_lines;
	int distance_traveled = 0;
	copy_bits(tetrimino->bits, new_bits);

	/* Keep descending until invalid position reached */
	while (valid_position(win, tetrimino, new_bits, well_contents))
	{
		for (i = 0; i < NUM_BITS; i++)
			new_bits[i].y++;
		distance_traveled++;
	}

	/* Invalid position reached, so go back up one unit before updating tetrimino bits */
	for (i = 0; i < NUM_BITS; i++)
		new_bits[i].y--;
	distance_traveled--;

	copy_bits(new_bits, tetrimino->bits);
	lock_tetrimino_into_well(tetrimino, well_contents, recent_hold);
	num_complete_lines = update_lines(win, tetrimino, difficulty, well_contents);

	/* Player gets more points by dropping tetriminos rather
	than letting them fall naturally */
	if (difficulty != INVALID_DIFF)
		SCORE += 2 * distance_traveled;

	init_tetrimino(tetrimino, get_rand_num(0, 6), well_contents, current_y_checkpoint);
	draw_well(win, tetrimino, well_contents);
	return num_complete_lines;
}

/* Shifts the coordinates of each bit in bits based on the given
direction */

void adjust_bits (COORDINATE_PAIR bits[NUM_BITS], int direction)
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

	for (i = 0; i < NUM_BITS; i++)
	{
		bits[i].y += delta_y;
		bits[i].x += delta_x;
	}
}

/* "Rotates" the coordinates of bits_to_rotate based on the given
pivot and direction */

void get_rotated_bits (COORDINATE_PAIR pivot, COORDINATE_PAIR bits_to_rotate[NUM_BITS], int direction)
{
	int temp, i;
	for (i = 0; i < NUM_BITS; i++)
	{
		bits_to_rotate[i].y -= pivot.y;
		bits_to_rotate[i].x -= pivot.x;

		if (direction == CLOCKWISE)
		{
			temp = bits_to_rotate[i].y; 
			bits_to_rotate[i].y =  -bits_to_rotate[i].x;
			bits_to_rotate[i].x = temp;
		}
		else // CNT_CLOCKWISE
		{
			temp = bits_to_rotate[i].x; 
			bits_to_rotate[i].x =  -bits_to_rotate[i].y;
			bits_to_rotate[i].y = temp;
		}

		bits_to_rotate[i].y += pivot.y;
		bits_to_rotate[i].x += pivot.x;
	}
}

/* Rotates the tetrimino in the specified direction. If the new location
is invalid (e.g. rotation occurs near an edge), then the tetrimino will 
attempt to shift itself into a valid position (while still preserving
the rotation). */

void rotate_tetrimino (WINDOW *win, TETRIMINO *tetrimino, int direction,
					  COORDINATE_PAIR well_contents[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH]) 
{
	/* Only rotate if the tetrimino is not an O piece */
	if (tetrimino->tetrimino_type != TETRIMINO_O)
	{
		COORDINATE_PAIR new_bits[NUM_BITS];
		COORDINATE_PAIR pivot;

		int i;
		int coords_out_of_y_bounds, coords_out_of_x_bounds;
		int delta_y, delta_x;
		int count_adjust = 0;

		pivot.y = tetrimino->bits[tetrimino->pivot_bit].y;
		pivot.x = tetrimino->bits[tetrimino->pivot_bit].x;

		copy_bits(tetrimino->bits, new_bits);
		get_rotated_bits(pivot, new_bits, direction);

		while (!valid_position(win, tetrimino, new_bits, well_contents))
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
					adjust_bits(new_bits, DOWN);
				else 
					adjust_bits(new_bits, UP);
			}

			if (coords_out_of_x_bounds)
			{
				/* Closer to the left? */
				if (abs(tetrimino->bits[0].x - WELL_L_BNDRY) < abs(tetrimino->bits[0].x - WELL_R_BNDRY))
					adjust_bits(new_bits, RIGHT);
				else 
					adjust_bits(new_bits, LEFT);
			}

			/* True if trying to rotate near another existing piece in the well */
			if (coords_out_of_y_bounds == 0 && coords_out_of_x_bounds == 0)
			{
				/* Attempt to move once in each direction from invalid position; */
				adjust_bits(new_bits, LEFT);
				if (valid_position(win, tetrimino, new_bits, well_contents)) break;
				adjust_bits(new_bits, RIGHT); adjust_bits(new_bits, RIGHT);
				if (valid_position(win, tetrimino, new_bits, well_contents)) break;
				adjust_bits(new_bits, LEFT); adjust_bits(new_bits, UP);
				if (valid_position(win, tetrimino, new_bits, well_contents)) break;
				adjust_bits(new_bits, DOWN); adjust_bits(new_bits, DOWN);
				if (valid_position(win, tetrimino, new_bits, well_contents)) break;

				/* All new positions are invalid, don't rotate at all */
				return;
			}
		}
		copy_bits(new_bits, tetrimino->bits);
	}
}

/* Attempts to initialize tetrimino based on tetrimino_id; 
if this fails (due to the tetrimino's spawning coordinates being occupied)
then GAME_OVER_FLAG is set */

void init_tetrimino (TETRIMINO *tetrimino, int tetrimino_id, 
					COORDINATE_PAIR well_contents[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH],
					int *current_y_checkpoint)
{
	tetrimino->tetrimino_type = tetrimino_id;
	tetrimino->pivot_bit = pivot_bits[tetrimino_id];

	int i;
	for (i = 0; i < NUM_BITS; i++)
	{
		if ((well_contents[init_y[tetrimino_id][i] - 1][init_x[tetrimino_id][i] - 1].value & A_CHARTEXT) != ' ')
		{
			well_contents[0][0].value = 'e'; // used to determine which player wins in versus 
			GAME_OVER_FLAG = 1;
			return;
		}

		tetrimino->bits[i].y = init_y[tetrimino_id][i];
		tetrimino->bits[i].x = init_x[tetrimino_id][i];

		/* Offset of 3 between ID number and COLOUR_PAIR number */
		tetrimino->bits[i].value = 'o' | COLOR_PAIR(tetrimino_id + 3);
	}

	*current_y_checkpoint = 0;
}

/* Updates well_contents with the value and coordinates of tetrimino's bits */

void lock_tetrimino_into_well(TETRIMINO *tetrimino, COORDINATE_PAIR well_contents[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH],
							 int *recent_hold)
{
	int row, col;
	int i;
	for (i = 0; i < NUM_BITS; i++)
	{
		row = tetrimino->bits[i].y - 1;
		col = tetrimino->bits[i].x - 1;
		well_contents[row][col].value = tetrimino->bits[i].value;
	}
	*recent_hold = 0;
}

/* Swaps the current tetrimino with the one "inside" the hold window 
and re-spawns the previous tetrimino - if hold window previously empty, then 
current tetrimino is held and a random one spawns */

void hold_tetrimino(WINDOW *hold_win, TETRIMINO *tetrimino,
					COORDINATE_PAIR well_contents[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH],
					int *current_y_checkpoint, int *recent_hold, int *currently_held_tetrimino_id)
{
	if (!(*recent_hold))
	{
		int old_id = update_hold(hold_win, tetrimino->tetrimino_type, currently_held_tetrimino_id);

		if (old_id != INVALID_ID)
			init_tetrimino(tetrimino, old_id, well_contents, current_y_checkpoint);
		else
			init_tetrimino(tetrimino, get_rand_num(0, 6), well_contents, current_y_checkpoint);

		*recent_hold = 1;
	}
}


/* Generate a random number ranging from lower to upper inclusive */

int get_rand_num (int lower, int upper)
{
	return rand() % (upper - lower + 1) + lower;
}

/* Checks if a line is "complete" - that is, all coordinates are occupied
by an 'o' */

int line_complete (int row, COORDINATE_PAIR well_contents[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH])
{
	int j;
	for (j = 0; j < WELL_CONTENTS_WIDTH; j++)
		if ((well_contents[row][j].value & A_CHARTEXT) != 'o')
			return FALSE;
	
	return TRUE;
}

/* Checks if a line is "empty" - that is, all coordinates are occupied
by a ' ' (space character) */

int line_empty (int row, COORDINATE_PAIR well_contents[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH])
{
	int j;
	for (j = 0; j < WELL_CONTENTS_WIDTH; j++)
		if (well_contents[row][j].value != ' ')
			return FALSE;
	
	return TRUE;
}

/* Makes a line empty */

void clear_line (int row, COORDINATE_PAIR well_contents[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH])
{
	int j;

	for (j = 0; j < WELL_CONTENTS_WIDTH; j++)
		well_contents[row][j].value = ' ';	
}

/* Handles line clearing every time a tetrimino locks into the well, highlighting all
complete lines before clearing them, determining the appropriate number of points to be 
rewarded, and how the rest of the well contents should be adjusted. Also adjusts the game 
delay every time the player levels up. Returns the number of complete lines.*/

int update_lines(WINDOW *win, TETRIMINO *tetrimino, int difficulty,
				  COORDINATE_PAIR well_contents[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH])
{
	
	int num_complete_lines = 0;
	int i, j;
	int complete = 0;
	int consec_complete_lines = 0;

	for (i = WELL_CONTENTS_HEIGHT - 1; i >= 0; i--)
	{
		if (line_empty(i, well_contents)) break;

		if (line_complete(i, well_contents))
		{
			for (j = 0; j < WELL_CONTENTS_WIDTH; j++)
				well_contents[i][j].value |= A_REVERSE; // "highlights" the complete rows
			
			complete = 1;
			consec_complete_lines++;
		}
		else
		{
			if (consec_complete_lines > 0 && difficulty != INVALID_DIFF)
				// Score determined by number of consecutive complete lines, current level, and difficulty
				SCORE += (base_pts_for_line_clears[consec_complete_lines - 1] * ((LINE_COUNT / 10) + 1 + difficulty));
			
			consec_complete_lines = 0;
		} 
	}

	draw_well(win, tetrimino, well_contents);
	if (complete)
		usleep(GAME_DELAY);  // pause briefly so player can see which lines were cleared

	for (i = WELL_CONTENTS_HEIGHT - 1; i >= 0; i--)
	{
		if (line_empty(i, well_contents)) break;

		if (line_complete(i, well_contents))
		{
			clear_line(i, well_contents);
			num_complete_lines++;
			LINE_COUNT++;

			// decrease game delay every level up (down to a minimum value, and assuming current mode is single)
			if (LINE_COUNT % 10 == 0 && GAME_DELAY >= MIN_DELAY && difficulty != INVALID_DIFF) 
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
			// copy line (i) to line (i + num_complete_lines)
			if (i + num_complete_lines != i) // this prevents lines from deleting themselves if they don't move
			{
				for (j = 0; j < WELL_CONTENTS_WIDTH; j++)
					well_contents[i + num_complete_lines][j].value = well_contents[i][j].value;
				
				clear_line(i, well_contents);
			}
		}

	}

	return num_complete_lines;
	
}