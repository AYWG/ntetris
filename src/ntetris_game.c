/* Gameplay-related functions for ntetris */

#include "ntetris.h"

static int base_pts_for_line_clears[] = {40, 100, 300, 1200};

static int init_y[NUM_TETRIMINOS][NUM_BITS] = {{1, 1, 1, 1},
										{1, 1, 1, 2},
										{1, 1, 1, 2},
										{1, 1, 2, 2},
										{1, 1, 2, 2},
										{1, 1, 1, 2},
										{1, 1, 2, 2}
										};

static int init_x[NUM_TETRIMINOS][NUM_BITS] = {{11, 12, 13, 14},
										{11, 12, 13, 13},
										{11, 12, 13, 11},
										{12, 13, 12, 13},
										{12, 13, 11, 12},
										{11, 12, 13, 12},
										{11, 12, 12, 13} 
										};

static int pivot_bits[NUM_TETRIMINOS] = {1, 1, 1, 0, 3, 1, 2};										

void game_state_init(GameState *state, EDifficulty difficulty, int mode)
{
	/* Generate random number seed*/
	srand((unsigned) time(NULL));

	/* Enable input from arrow keys */
	keypad(stdscr, TRUE);

	state->difficulty = difficulty;
	state->mode = mode;

	if (mode == SINGLE) {
		switch(difficulty)
		{
			case CASUAL: state->game_delay = CASUAL_INIT_DELAY; break;
			case INTERMEDIATE: state->game_delay = INTERMEDIATE_INIT_DELAY; break;
			case EXPERT: state->game_delay = EXPERT_INIT_DELAY; break;
		}
	}
	else if (mode == VERSUS) {
		state->game_delay = INTERMEDIATE_INIT_DELAY;
	}

	state->currently_held_tetrimino[PLAYER_1] = INVALID_ID;
	state->currently_held_tetrimino[PLAYER_2] = INVALID_ID;

	// Init well contents
	int i, j;
	for (i = 0; i < WELL_CONTENTS_HEIGHT; i++)
	{
		for (j = 0; j < WELL_CONTENTS_WIDTH; j++)
		{
			state->well_contents[PLAYER_1][i][j].y = i + 1;
			state->well_contents[PLAYER_1][i][j].x = j + 1;
			state->well_contents[PLAYER_1][i][j].value = ' ';

			if (mode == VERSUS) {
				state->well_contents[PLAYER_2][i][j].y = i + 1;
				state->well_contents[PLAYER_2][i][j].x = j + 1;
				state->well_contents[PLAYER_2][i][j].value = ' ';
			}
		}
	}
}

void reset_game_state(GameState *state)
{
	state->currently_held_tetrimino[PLAYER_1] = INVALID_ID;
	state->currently_held_tetrimino[PLAYER_2] = INVALID_ID;
	state->difficulty = INVALID_DIFFICULTY;
	state->game_over_flag = NOT_OVER;
	state->has_held_recently[PLAYER_1] = FALSE;
	state->has_held_recently[PLAYER_2] = FALSE;
	state->line_count = 0;
	state->score = 0;
	state->current_y_checkpoint[PLAYER_1] = 0;
	state->current_y_checkpoint[PLAYER_2] = 0;
	state->garbage_counter[PLAYER_1] = 0;
	state->garbage_counter[PLAYER_2] = 0;
}

/* Determines whether cp_1 is located at the same coordinates as cp_2. */

static int equal_coords (COORDINATE_PAIR cp_1, COORDINATE_PAIR cp_2)
{
	return (cp_1.x == cp_2.x) && (cp_1.y == cp_2.y);
}

/* Determines whether each bit in bits_1 has the same coordinates as the corresponding bit
in bits_2 */

static int equal_bits (COORDINATE_PAIR bits_1[NUM_BITS], COORDINATE_PAIR bits_2[NUM_BITS])
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

/* Checks if a line is "complete" - that is, all coordinates are occupied
by an 'o' */

static int line_complete (int row, COORDINATE_PAIR well_contents[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH])
{
	int j;
	for (j = 0; j < WELL_CONTENTS_WIDTH; j++)
		if ((well_contents[row][j].value & A_CHARTEXT) != 'o')
			return FALSE;
	
	return TRUE;
}



/* Makes a line empty */

static void clear_line (int row, COORDINATE_PAIR well_contents[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH])
{
	int j;

	for (j = 0; j < WELL_CONTENTS_WIDTH; j++)
		well_contents[row][j].value = ' ';	
}

/* Determines whether the given tetrimino coords are outside the boundaries
of the given player's well */

static int out_of_boundaries (GameState *state, EPlayer player_id, COORDINATE_PAIR tetr_coords)
{
	return (tetr_coords.y < 1 || tetr_coords.y > state->well_max_y[player_id] - 2 ||
			tetr_coords.x < 1 || tetr_coords.x > state->well_max_x[player_id] - 2);
}

/* Determines if new_bits is a valid array of bits for the tetrimino within
the given player's well*/

int valid_position (GameState *state, EPlayer player_id, COORDINATE_PAIR new_bits[NUM_BITS])
{
	int row, col;
	int i;

	for (i = 0; i < NUM_BITS; i++)
	{
		/* Check boundaries */
		if (out_of_boundaries(state, player_id, new_bits[i]))
			return FALSE;
		
		row = new_bits[i].y - 1;
		col = new_bits[i].x - 1;

		/* Valid coordinates in the well are those unoccupied or occupied by the 
		tetrimino's "ghost" */
		if ((state->well_contents[player_id][row][col].value & A_CHARTEXT != ' ') &&
			(state->well_contents[player_id][row][col].value & A_ATTRIBUTES) != A_DIM)
			return FALSE;
		
	}
	return TRUE;
}

/* Shifts the coordinates of each bit in bits based on the given
direction */

static void adjust_bits (COORDINATE_PAIR bits[NUM_BITS], EDirection direction)
{
	int delta_y = 0, delta_x = 0;
	int i;

	switch(direction)
	{
		case UP: delta_y = -1; delta_x = 0; break;
		case DOWN: delta_y = 1; delta_x = 0; break;
		case LEFT: delta_y = 0; delta_x = -1; break;
		case RIGHT: delta_y = 0; delta_x = 1; break;
		default: break;
	}

	for (i = 0; i < NUM_BITS; i++)
	{
		bits[i].y += delta_y;
		bits[i].x += delta_x;
	}
}

/* Updates the player's tetrimino's location based on the given direction, if possible. 
Allowed directions include: left, right, and down. If the new location is invalid
(e.g. something is in the way) then nothing happens */

void move_tetrimino (GameState *state, EPlayer player_id, EDirection direction) 
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
		default: break;
	}

	for (i = 0; i < NUM_BITS; i++)
	{
		new_bits[i].y = state->tetrimino[player_id].bits[i].y + delta_y;
		new_bits[i].x = state->tetrimino[player_id].bits[i].x + delta_x;
	}
	
	/* if the new coordinates are valid, update position */

	if (valid_position(state, player_id, new_bits))
	{
		copy_bits(new_bits, state->tetrimino[player_id].bits);
		/* "soft" drops award 1 point */
		if (direction == DOWN) 
			state->score++;
	}
}

/* Instantly move the player's tetrimino to where it would go if 
just fell naturally down the well from its current position,
then lock it into the well. Returns the number of lines cleared
as a result of dropping the tetrimino */

int drop_tetrimino (GameState *state, EPlayer player_id)
{
	COORDINATE_PAIR new_bits[NUM_BITS];
	int i;
	int num_complete_lines;
	int distance_traveled = 0;
	copy_bits(state->tetrimino[player_id].bits, new_bits);

	/* Keep descending until invalid position reached */
	while (valid_position(state, player_id, new_bits))
	{
		for (i = 0; i < NUM_BITS; i++)
			new_bits[i].y++;
		distance_traveled++;
	}

	/* Invalid position reached, so go back up one unit before updating tetrimino bits */
	for (i = 0; i < NUM_BITS; i++)
		new_bits[i].y--;
	distance_traveled--;

	copy_bits(new_bits, state->tetrimino[player_id].bits);
	lock_tetrimino_into_well(state, player_id);
	num_complete_lines = update_lines(state, player_id);

	/* Player gets more points by dropping tetriminos rather
	than letting them fall naturally */
	if (state->difficulty != INVALID_DIFFICULTY)
		state->score += 2 * distance_traveled;

	init_tetrimino(state, player_id, get_rand_num(0, 6));
	return num_complete_lines;
}

/* "Rotates" the coordinates of bits_to_rotate based on the given
pivot and direction */

void get_rotated_bits (COORDINATE_PAIR pivot, COORDINATE_PAIR bits_to_rotate[NUM_BITS], EDirection direction)
{
	int temp, i;
	for (i = 0; i < NUM_BITS; i++)
	{
		bits_to_rotate[i].y -= pivot.y;
		bits_to_rotate[i].x -= pivot.x;

		if (direction == CW)
		{
			temp = bits_to_rotate[i].y; 
			bits_to_rotate[i].y =  -bits_to_rotate[i].x;
			bits_to_rotate[i].x = temp;
		}
		else if (direction == CCW)
		{
			temp = bits_to_rotate[i].x; 
			bits_to_rotate[i].x =  -bits_to_rotate[i].y;
			bits_to_rotate[i].y = temp;
		}

		bits_to_rotate[i].y += pivot.y;
		bits_to_rotate[i].x += pivot.x;
	}
}

/* Rotates the player's tetrimino in the specified direction. If the new location
is invalid (e.g. rotation occurs near an edge), then the tetrimino will 
attempt to shift itself into a valid position (while still preserving
the rotation). */

void rotate_tetrimino (GameState *state, EPlayer player_id, EDirection direction) 
{
	TETRIMINO *tetrimino = &state->tetrimino[player_id];
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

		while (!valid_position(state, player_id, new_bits))
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
				if (valid_position(state, player_id, new_bits)) break;
				adjust_bits(new_bits, RIGHT); adjust_bits(new_bits, RIGHT);
				if (valid_position(state, player_id, new_bits)) break;
				adjust_bits(new_bits, LEFT); adjust_bits(new_bits, UP);
				if (valid_position(state, player_id, new_bits)) break;
				adjust_bits(new_bits, DOWN); adjust_bits(new_bits, DOWN);
				if (valid_position(state, player_id, new_bits)) break;

				/* All new positions are invalid, don't rotate at all */
				return;
			}
		}
		copy_bits(new_bits, tetrimino->bits);
	}
}

/* Attempts to initialize player's tetrimino based on tetrimino_id; 
if this fails (due to the tetrimino's spawning coordinates being occupied)
then GAME_OVER_FLAG is set */

void init_tetrimino (GameState *state, EPlayer player_id, ETetrimino tetrimino_id)
{
	state->tetrimino[player_id].tetrimino_type = tetrimino_id;
	state->tetrimino[player_id].pivot_bit = pivot_bits[tetrimino_id];

	int i;
	for (i = 0; i < NUM_BITS; i++)
	{
		if ((state->well_contents[player_id][init_y[tetrimino_id][i] - 1][init_x[tetrimino_id][i] - 1].value & A_CHARTEXT) != ' ')
		{
			state->game_over_flag = player_id == PLAYER_1 ? PLAYER_1_LOST : PLAYER_2_LOST;
			return;
		}

		state->tetrimino[player_id].bits[i].y = init_y[tetrimino_id][i];
		state->tetrimino[player_id].bits[i].x = init_x[tetrimino_id][i];

		/* Offset of 3 between ID number and COLOUR_PAIR number */
		state->tetrimino[player_id].bits[i].value = 'o' | COLOR_PAIR(tetrimino_id + 3);
	}

	state->current_y_checkpoint[player_id] = 0;
}

/* Updates player's well_contents with the value and coordinates of its tetrimino's bits */

void lock_tetrimino_into_well(GameState *state, EPlayer player_id)
{
	int row, col;
	int i;
	for (i = 0; i < NUM_BITS; i++)
	{
		row = state->tetrimino[player_id].bits[i].y - 1;
		col = state->tetrimino[player_id].bits[i].x - 1;
		state->well_contents[player_id][row][col].value = state->tetrimino[player_id].bits[i].value;
	}
	state->has_held_recently[player_id] = FALSE;
}

/* Swaps the current tetrimino with the one "inside" the hold window 
and re-spawns the previous tetrimino - if hold window previously empty, then 
current tetrimino is held and a random one spawns */

void hold_tetrimino(GameState *state, EPlayer player_id)
{
	if (!(state->has_held_recently[player_id]))
	{
		ETetrimino held_id = state->currently_held_tetrimino[player_id];
		ETetrimino new_hold_id = state->tetrimino[player_id].tetrimino_type; 
		if (held_id == INVALID_ID)
			init_tetrimino(state, player_id, get_rand_num(0, 6));
		else
			init_tetrimino(state, player_id, held_id);

		state->currently_held_tetrimino[player_id] = new_hold_id;
		state->has_held_recently[player_id] = TRUE;
	}
}


/* Generate a random number ranging from lower to upper inclusive */

int get_rand_num (int lower, int upper)
{
	return rand() % (upper - lower + 1) + lower;
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

/* Handles line clearing every time a tetrimino locks into the well, highlighting all
complete lines before clearing them, determining the appropriate number of points to be 
rewarded, and how the rest of the well contents should be adjusted. Also adjusts the game 
delay every time the player levels up. Returns the number of complete lines.*/

int update_lines(GameState *state, EPlayer player_id)
{
	
	int num_complete_lines = 0;
	int i, j;
	int complete = 0;
	int consec_complete_lines = 0;

	for (i = WELL_CONTENTS_HEIGHT - 1; i >= 0; i--)
	{
		if (line_empty(i, state->well_contents[player_id])) break;

		if (line_complete(i, state->well_contents[player_id]))
		{
			for (j = 0; j < WELL_CONTENTS_WIDTH; j++)
				state->well_contents[player_id][i][j].value |= A_REVERSE; // "highlights" the complete rows
			
			complete = 1;
			consec_complete_lines++;
		}
		else
		{
			if (consec_complete_lines > 0 && state->difficulty != INVALID_DIFFICULTY)
				// Score determined by number of consecutive complete lines, current level, and difficulty
				state->score += (base_pts_for_line_clears[consec_complete_lines - 1] * ((state->line_count / 10) + 1 + state->difficulty));
			
			consec_complete_lines = 0;
		} 
	}

	if (complete)
		usleep(state->game_delay);  // pause briefly so player can see which lines were cleared

	for (i = WELL_CONTENTS_HEIGHT - 1; i >= 0; i--)
	{
		if (line_empty(i, state->well_contents[player_id])) break;

		if (line_complete(i, state->well_contents[player_id]))
		{
			clear_line(i, state->well_contents[player_id]);
			num_complete_lines++;
			state->line_count++;

			// decrease game delay every level up (down to a minimum value, and assuming current mode is single)
			if (state->line_count % 10 == 0 && state->game_delay >= MIN_DELAY && state->difficulty != INVALID_DIFFICULTY) 
			{ 
				switch(state->difficulty)
				{
					case CASUAL:
						state->game_delay -= CASUAL_INIT_DELAY / 20; break;
					case INTERMEDIATE:
						state->game_delay -= INTERMEDIATE_INIT_DELAY / 20; break;
					case EXPERT:
						state->game_delay -= EXPERT_INIT_DELAY / 20; break;
				}
			}
		}
		else
		{
			// copy line (i) to line (i + num_complete_lines)
			if (i + num_complete_lines != i) // this prevents lines from deleting themselves if they don't move
			{
				for (j = 0; j < WELL_CONTENTS_WIDTH; j++)
					state->well_contents[player_id][i + num_complete_lines][j].value = state->well_contents[player_id][i][j].value;
				
				clear_line(i, state->well_contents[player_id]);
			}
		}

	}

	return num_complete_lines;
	
}