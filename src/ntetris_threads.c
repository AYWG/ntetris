#include "ntetris.h"

static pthread_mutex_t tetrimino_lock[] = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER};
static pthread_mutex_t garbage_line_counter_lock[] = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER};

/* Checks if the given input is an element of controls */

int is_input_useful(int input, int controls[NUM_CONTROLS])
{
	int i;
	for (i = 0; i < NUM_CONTROLS; i++)
		if (controls[i] == input)
			return TRUE;
	
	return FALSE;
}

/* Adds garbage lines to well_contents, resets garbage_counter, and adds num_complete_lines to
other_garbage_counter. Note that the number of garbage lines added to well_contents is reduced
by num_complete_lines */

void add_garbage(GameState *state, EPlayer from_player, EPlayer to_player, int num_complete_lines)
{
	pthread_mutex_lock(&(garbage_line_counter_lock[to_player]));

	int i, j;
	int highest_nonempty_row;

	/* Decrement counter by number of complete lines; minimum counter value is 0 */
	for (i = 0; i < num_complete_lines; i++)
	{
		if (state->garbage_counter[to_player] == 0) break;
		state->garbage_counter[to_player]--;
	}

	/* Add the remaining counter value to the well as garbage lines */
	for (i = WELL_CONTENTS_HEIGHT - 1; i >= 0; i--)
		if (line_empty(i, state->well_contents[to_player])) break;

	highest_nonempty_row = i + 1;

	for (i = highest_nonempty_row; i < WELL_CONTENTS_HEIGHT; i++)
	{
		if (i - state->garbage_counter[to_player] != i)
		{
			for (j = 0; j < WELL_CONTENTS_WIDTH; j++)
				state->well_contents[to_player][i - state->garbage_counter[to_player]][j].value = state->well_contents[to_player][i][j].value;
		}
	}

	for (i = WELL_CONTENTS_HEIGHT - 1; i > WELL_CONTENTS_HEIGHT - 1 - state->garbage_counter[to_player]; i--)
	{
		/* Garbage lines are white */
		for (j = 0; j < WELL_CONTENTS_WIDTH; j++)
			state->well_contents[to_player][i][j].value = 'o' | COLOR_PAIR(MAIN_TEXT_COLOR_PAIR);

		/* One random bit missing in the garbage line (so that it doesn't automatically disappear) */
		state->well_contents[to_player][i][get_rand_num(0, WELL_CONTENTS_WIDTH - 1)].value = ' ';
	}

	state->garbage_counter[to_player] = 0;
	// update_garbage_line_counter(garbage_win, garbage_counter);
	pthread_mutex_unlock(&(garbage_line_counter_lock[to_player]));

	pthread_mutex_lock(&(garbage_line_counter_lock[from_player]));

	/* Increment other counter by number of_complete lines; maximum counter value is 5 */
	for (i = 0; i < num_complete_lines; i++)
	{
		if (state->garbage_counter[from_player] == 5) break;
		state->garbage_counter[from_player]++;
	}

	// update_garbage_line_counter(other_garbage_win, other_garbage_counter);
	pthread_mutex_unlock(&(garbage_line_counter_lock[from_player]));
}


/* Thread responsible updating the GUI */
void *run_gui (void *ui)
{
	GUI *gui = (GUI *) ui;
	int mode = gui->state->mode;

	while(TRUE)
	{
		usleep(gui->refresh_delay);

		update_well(gui, PLAYER_1);
		update_hold(gui, PLAYER_1, gui->state->currently_held_tetrimino[PLAYER_1]);

		if (mode == SINGLE) {
			update_line_count(gui, PLAYER_1);
			update_level(gui, PLAYER_1);
			update_score(gui, PLAYER_1);
		}

		if (mode == VERSUS) {
			update_garbage_line_counter(gui, PLAYER_1);

			update_well(gui, PLAYER_2);
			update_hold(gui, PLAYER_2, gui->state->currently_held_tetrimino[PLAYER_2]);
			update_garbage_line_counter(gui, PLAYER_2);
		}
		doupdate();
	}
}

/* Thread responsible for moving the tetrimino down one unit at a time with GAME_DELAY pauses. */

void *periodic_thread (void *arguments)
{
	ThreadArgs *args = (ThreadArgs *) arguments;
	GameState *state = (GameState *) args->state;
	EPlayer player_id = args->player_id;
	while(TRUE)
	{
		usleep(state->game_delay); 
		
		pthread_mutex_lock(&(tetrimino_lock[player_id]));
		move_tetrimino(state, player_id, DOWN);
		pthread_mutex_unlock(&(tetrimino_lock[player_id]));
	}
}

/* Thread responsible for "locking in" a tetrimino into the well */

void *lock_in_thread (void *arguments)
{
	ThreadArgs *args = (ThreadArgs *) arguments;
	GameState *state = args->state;
	EPlayer player_id = args->player_id;
	EPlayer other_player_id = player_id == PLAYER_1 ? PLAYER_2 : PLAYER_1;
	COORDINATE_PAIR current_bits[NUM_BITS];
	int pivot_bit = state->tetrimino[PLAYER_1].pivot_bit;
	int num_complete_lines;

	while(TRUE)
	{
		/* Repeatedly check if the tetrimino has fallen past a "checkpoint" that is calculated 
		based on its current position. If it does not fall past the checkpoint within three GAME_DELAYs,
		then it likely cannot fall any further and should be locked in. */

		usleep((state->game_delay));
		if (get_y_checkpoint(state->tetrimino[player_id].bits) > (state->current_y_checkpoint[player_id]))
		{
			state->current_y_checkpoint[player_id] = get_y_checkpoint(state->tetrimino[player_id].bits);
			continue;
		}
		usleep((state->game_delay));
		if (get_y_checkpoint(state->tetrimino[player_id].bits) > (state->current_y_checkpoint[player_id]))
		{
			state->current_y_checkpoint[player_id] = get_y_checkpoint(state->tetrimino[player_id].bits);
			continue;
		}
		usleep((state->game_delay));
		if (get_y_checkpoint(state->tetrimino[player_id].bits) > (state->current_y_checkpoint[player_id]))
		{
			state->current_y_checkpoint[player_id] = get_y_checkpoint(state->tetrimino[player_id].bits);
			continue;
		}

		pthread_mutex_lock(&(tetrimino_lock[player_id]));
		lock_tetrimino_into_well(state, player_id);
		num_complete_lines = update_lines(state, player_id);

		if (state->mode == VERSUS)
		{
			add_garbage(state, other_player_id, player_id, num_complete_lines);
		}
		
		init_tetrimino(state, player_id, get_rand_num(0, 6));
		pthread_mutex_unlock(&(tetrimino_lock[player_id]));
	}
}

/* Top-level thread for running single player mode. */

void *play_ntetris_single (void *game_state) 
{
	GameState *state = (GameState *) game_state;

	init_tetrimino(state, PLAYER_1, get_rand_num(0, 6));

	pthread_t periodic_t;
	pthread_t lock_in_t;

	ThreadArgs args;
	args.state = state;
	args.player_id = PLAYER_1;
	
	if (pthread_create(&periodic_t, NULL, &periodic_thread, &args))
		printf("Could not run periodic thread\n");
	if (pthread_create(&lock_in_t, NULL, &lock_in_thread, &args))
		printf("Could not run lock in thread\n");

	int ch;

	/* Cause getch() calls to wait for user input for 0.1 s (rather than wait indefinitely). 
	If no input is received, getch() returns ERR. This is done to ensure that the game ends
	promptly when the game over flag is set (instead of waiting for user input). */
	halfdelay(1);

	while ((ch = getch()) != QUIT_KEY)
	{
		if (ch != ERR)
		{
			/* Acquire tetrimino lock before we do anything to the tetrimino */
			pthread_mutex_lock(&tetrimino_lock[PLAYER_1]);
			switch(ch)
			{
				case KEY_LEFT:
					move_tetrimino(state, PLAYER_1, LEFT); break;
				case KEY_RIGHT:
					move_tetrimino(state, PLAYER_1, RIGHT); break;
				case KEY_DOWN:
					move_tetrimino(state, PLAYER_1, DOWN); break;
				case KEY_UP:
					drop_tetrimino(state, PLAYER_1); break;
				case P_KEY:
					rotate_tetrimino(state, PLAYER_1, CW); break;
				case O_KEY:
					rotate_tetrimino(state, PLAYER_1, CCW); break;
				case ENTER_KEY:
					hold_tetrimino(state, PLAYER_1); break;
			}
			/* Done, so release tetrimino lock */
			pthread_mutex_unlock(&(tetrimino_lock[PLAYER_1]));

			/* Wait a bit so other threads can acquire lock */
			usleep(random() % STALL);
		}
		else 
		{
			usleep(random() % STALL);
		}
		if (state->game_over_flag) break;
	}
	/* getch() calls are now blocking as usual */
	nocbreak();
	cbreak();

	/* This point is reached if user presses QUIT_KEY or GAME_OVER_FLAG is set; 
	need to stop the other threads */
	pthread_cancel(periodic_t);
	pthread_cancel(lock_in_t);
	if (pthread_join(periodic_t, NULL))
		printf("Could not properly terminate periodic thread\n");
	if (pthread_join(lock_in_t, NULL))
		printf("Could not properly terminate lock in thread\n");
}

/* Top-level thread for running versus mode. */

void *play_ntetris_versus (void *game_state)
{
	GameState *state = (GameState *) game_state;

	int controls_p1[NUM_CONTROLS] = {KEY_LEFT, KEY_RIGHT, KEY_DOWN, KEY_UP, P_KEY, O_KEY, ENTER_KEY};
	int controls_p2[NUM_CONTROLS] = {A_KEY, D_KEY, S_KEY, W_KEY, G_KEY, F_KEY, SPACE_KEY};

	init_tetrimino(state, PLAYER_1, get_rand_num(0, 6));
	init_tetrimino(state, PLAYER_2, get_rand_num(0, 6));

	pthread_t periodic_t_p1, periodic_t_p2;
	pthread_t lock_in_t_p1, lock_in_t_p2;
	
	ThreadArgs args_p1;
	args_p1.state = state;
	args_p1.player_id = PLAYER_1;

	ThreadArgs args_p2;
	args_p2.state = state;
	args_p2.player_id = PLAYER_2;

	if (pthread_create(&periodic_t_p1, NULL, &periodic_thread, &args_p1))
		printf("Could not run periodic thread\n");
	if (pthread_create(&lock_in_t_p1, NULL, &lock_in_thread, &args_p1))
		printf("Could not run lock in thread\n");

	if (pthread_create(&periodic_t_p2, NULL, &periodic_thread, &args_p2))
		printf("Could not run periodic thread\n");
	if (pthread_create(&lock_in_t_p2, NULL, &lock_in_thread, &args_p2))
		printf("Could not run lock in thread\n");

	int ch;
	int num_complete_lines_1;
	int num_complete_lines_2;

	/* Cause getch() calls to wait for user input for 0.1 s (rather than wait indefinitely). 
	If no input is received, getch() returns ERR*/
	halfdelay(1);

	while ((ch = getch()) != QUIT_KEY)
	{
		if (ch != ERR)
		{
			/* Since this loop checks for input from both players, need to check if
			a given input has any "usefulness" to a specific player (to avoid unnecessary computations) */
			if (is_input_useful(ch, controls_p1))
			{
				pthread_mutex_lock(&tetrimino_lock[PLAYER_1]);

				switch(ch)
				{
					case KEY_LEFT:
						move_tetrimino(state, PLAYER_1, LEFT); break;
					case KEY_RIGHT:
						move_tetrimino(state, PLAYER_1, RIGHT); break;
					case KEY_DOWN:
						move_tetrimino(state, PLAYER_1, DOWN); break;
					case KEY_UP:
						num_complete_lines_1 = drop_tetrimino(state, PLAYER_1); break;
					case P_KEY:
						rotate_tetrimino(state, PLAYER_1, CW); break;
					case O_KEY:
						rotate_tetrimino(state, PLAYER_1, CCW); break;
					case ENTER_KEY:
						hold_tetrimino(state, PLAYER_1); break;
				}
				if (ch == KEY_UP) // if tetrimino was dropped
					add_garbage(state, PLAYER_2, PLAYER_1, num_complete_lines_1);

				pthread_mutex_unlock(&tetrimino_lock[PLAYER_1]);
				usleep(random() % STALL);
			}

			else if (is_input_useful(ch, controls_p2))
			{
				pthread_mutex_lock(&tetrimino_lock[PLAYER_2]);

				switch(ch)
				{
					case A_KEY:
						move_tetrimino(state, PLAYER_2, LEFT); break;
					case D_KEY:
						move_tetrimino(state, PLAYER_2, RIGHT); break;
					case S_KEY:
						move_tetrimino(state, PLAYER_2, DOWN); break;
					case W_KEY:
						num_complete_lines_2 = drop_tetrimino(state, PLAYER_2); break;
					case G_KEY:
						rotate_tetrimino(state, PLAYER_2, CW); break;
					case F_KEY:
						rotate_tetrimino(state, PLAYER_2, CCW); break;
					case SPACE_KEY:
						hold_tetrimino(state, PLAYER_2); break;
				}
				if (ch == W_KEY) // if tetrimino was dropped
					add_garbage(state, PLAYER_1, PLAYER_2, num_complete_lines_2);

				pthread_mutex_unlock(&(tetrimino_lock[PLAYER_2]));
				usleep(random() % STALL);
			}
		}
		else 
		{
			usleep(random() % STALL);
		}
		if (state->game_over_flag) break;
	}
	/* getch() calls are now blocking as usual */
	nocbreak();
	cbreak();

	/* This point is reached if user presses QUIT_KEY or GAME_OVER_FLAG is set; 
	need to stop the other threads */
	pthread_cancel(periodic_t_p1);
	pthread_cancel(lock_in_t_p1);
	pthread_cancel(periodic_t_p2);
	pthread_cancel(lock_in_t_p2);
	
	if (pthread_join(periodic_t_p1, NULL))
		printf("Could not properly terminate periodic thread\n");
	if (pthread_join(lock_in_t_p1, NULL))
		printf("Could not properly terminate lock in thread\n");
	if (pthread_join(periodic_t_p2, NULL))
		printf("Could not properly terminate periodic thread\n");
	if (pthread_join(lock_in_t_p2, NULL))
		printf("Could not properly terminate lock in thread\n");
}