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

void add_garbage(WINDOW *garbage_win, WINDOW *other_garbage_win, int num_complete_lines, 
				int lock_num, int *garbage_counter, int *other_garbage_counter,
				COORDINATE_PAIR well_contents[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH])
{
	pthread_mutex_lock(&(garbage_line_counter_lock[lock_num]));

	int i, j;
	int highest_nonempty_row;

	/* Decrement counter by number of complete lines; minimum counter value is 0 */
	for (i = 0; i < num_complete_lines; i++)
	{
		if (*garbage_counter == 0) break;
		(*garbage_counter)--;
	}

	/* Add the remaining counter value to the well as garbage lines */
	for (i = WELL_CONTENTS_HEIGHT - 1; i >= 0; i--)
		if (line_empty(i, well_contents)) break;

	highest_nonempty_row = i + 1;

	for (i = highest_nonempty_row; i < WELL_CONTENTS_HEIGHT; i++)
	{
		if (i - *garbage_counter != i)
		{
			for (j = 0; j < WELL_CONTENTS_WIDTH; j++)
				well_contents[i - *garbage_counter][j].value = well_contents[i][j].value;
		}
	}

	for (i = WELL_CONTENTS_HEIGHT - 1; i > WELL_CONTENTS_HEIGHT - 1 - *garbage_counter; i--)
	{
		/* Garbage lines are white */
		for (j = 0; j < WELL_CONTENTS_WIDTH; j++)
			well_contents[i][j].value = 'o' | COLOR_PAIR(MAIN_TEXT_COLOR_PAIR);

		/* One random bit missing in the garbage line (so that it doesn't automatically disappear) */
		well_contents[i][get_rand_num(0, WELL_CONTENTS_WIDTH - 1)].value = ' ';
	}

	*garbage_counter = 0;
	update_garbage_line_counter(garbage_win, garbage_counter);
	pthread_mutex_unlock(&(garbage_line_counter_lock[lock_num]));

	pthread_mutex_lock(&(garbage_line_counter_lock[!lock_num]));

	/* Increment other counter by number of_complete lines; maximum counter value is 5 */
	for (i = 0; i < num_complete_lines; i++)
	{
		if (*other_garbage_counter == 5) break;
		(*other_garbage_counter)++;
	}

	update_garbage_line_counter(other_garbage_win, other_garbage_counter);
	pthread_mutex_unlock(&(garbage_line_counter_lock[!lock_num]));
}


/* Thread responsible updating the GUI */
void *run_gui (void *ui)
{
	GUI *gui = (GUI *) ui;

	while(TRUE)
	{
		usleep(gui->refresh_delay);

		update_well(gui, PLAYER_1);
		update_hold(gui, PLAYER_1, gui->state->currently_held_tetrimino[PLAYER_1]);
		update_line_count(gui, PLAYER_1);
		update_level(gui, PLAYER_1);
		update_score(gui, PLAYER_1);
		doupdate();
	}
}

/* Thread responsible for moving the tetrimino down one unit at a time with GAME_DELAY pauses. */

void *periodic_thread (void *game_state)
{
	GameState *state = (GameState *) game_state;
	while(TRUE)
	{
		usleep(state->game_delay); 
		
		pthread_mutex_lock(&(tetrimino_lock[PLAYER_1]));
		move_tetrimino(state, PLAYER_1, DOWN);

		// if (args->mode == SINGLE)
		// 	update_score(args->win[SCORE_ID]);

		// draw_well(args->win[WELL_ID], args->tetrimino, args->well_contents);
		pthread_mutex_unlock(&(tetrimino_lock[PLAYER_1]));		
	}
}

/* Thread responsible for "locking in" a tetrimino into the well */

void *lock_in_thread (void *game_state)
{
	GameState *state = (GameState *) game_state;
	COORDINATE_PAIR current_bits[NUM_BITS];
	int pivot_bit = state->tetrimino[PLAYER_1].pivot_bit;
	int num_complete_lines;

	while(TRUE)
	{
		/* Repeatedly check if the tetrimino has fallen past a "checkpoint" that is calculated 
		based on its current position. If it does not fall past the checkpoint within three GAME_DELAYs,
		then it likely cannot fall any further and should be locked in. */

		usleep((state->game_delay));
		if (get_y_checkpoint(state->tetrimino[PLAYER_1].bits) > (state->current_y_checkpoint[PLAYER_1]))
		{
			state->current_y_checkpoint[PLAYER_1] = get_y_checkpoint(state->tetrimino[PLAYER_1].bits);
			continue;
		}
		usleep((state->game_delay));
		if (get_y_checkpoint(state->tetrimino[PLAYER_1].bits) > (state->current_y_checkpoint[PLAYER_1]))
		{
			state->current_y_checkpoint[PLAYER_1] = get_y_checkpoint(state->tetrimino[PLAYER_1].bits);
			continue;
		}
		usleep((state->game_delay));
		if (get_y_checkpoint(state->tetrimino[PLAYER_1].bits) > (state->current_y_checkpoint[PLAYER_1]))
		{
			state->current_y_checkpoint[PLAYER_1] = get_y_checkpoint(state->tetrimino[PLAYER_1].bits);
			continue;
		}

		pthread_mutex_lock(&(tetrimino_lock[PLAYER_1]));
		lock_tetrimino_into_well(state, PLAYER_1);
		num_complete_lines = update_lines(state, PLAYER_1);

		if (state->mode == VERSUS)
		{
			// add_garbage(args->win[GARBAGE_ID], args->win[OTHER_GARBAGE_ID], num_complete_lines, 
			// 		   args->lock_num, args->garbage_counter, args->other_garbage_counter, args->well_contents);
		}
		
		init_tetrimino(state, PLAYER_1, get_rand_num(0, 6));
		pthread_mutex_unlock(&(tetrimino_lock[PLAYER_1]));
	}
}

/* Top-level thread for running single player mode. */

void *play_ntetris_single (void *game_state) 
{
	GameState *state = (GameState *) game_state;

	init_tetrimino(state, PLAYER_1, get_rand_num(0, 6));

	pthread_t periodic_t;
	pthread_t lock_in_t;
	
	if (pthread_create(&periodic_t, NULL, &periodic_thread, state))
		printf("Could not run periodic thread\n");
	if (pthread_create(&lock_in_t, NULL, &lock_in_thread, state))
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
			if (state->game_over_flag) break;
			usleep(random() % STALL);
		}
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

	init_tetrimino(tetrimino_p1, get_rand_num(0, 6), well_contents_p1, &CURRENT_Y_CHECKPOINT);
	init_tetrimino(tetrimino_p2, get_rand_num(0, 6), well_contents_p2, &CURRENT_Y_CHECKPOINT_2);

	pthread_t periodic_t_p1, periodic_t_p2;
	pthread_t lock_in_t_p1, lock_in_t_p2;
	pthread_t get_user_input_versus_t;

	if (pthread_create(&periodic_t_p1, NULL, &periodic_thread, args_p1))
		printf("Could not run periodic thread\n");
	if (pthread_create(&lock_in_t_p1, NULL, &lock_in_thread, args_p1))
		printf("Could not run lock in thread\n");

	if (pthread_create(&periodic_t_p2, NULL, &periodic_thread, args_p2))
		printf("Could not run periodic thread\n");
	if (pthread_create(&lock_in_t_p2, NULL, &lock_in_thread, args_p2))
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
				pthread_mutex_lock(&tetrimino_lock[args_p1->lock_num]);

				switch(ch)
				{
					case KEY_LEFT:
						move_tetrimino(well_win_p1, tetrimino_p1, LEFT, well_contents_p1); break;
					case KEY_RIGHT:
						move_tetrimino(well_win_p1, tetrimino_p1, RIGHT, well_contents_p1); break;
					case KEY_DOWN:
						move_tetrimino(well_win_p1, tetrimino_p1, DOWN, well_contents_p1); break;
					case KEY_UP:
						num_complete_lines_1 = drop_tetrimino(well_win_p1, tetrimino_p1, args_p1->difficulty, 
											 well_contents_p1, &CURRENT_Y_CHECKPOINT, &RECENT_HOLD); break;
					case P_KEY:
						rotate_tetrimino(well_win_p1, tetrimino_p1, CW, well_contents_p1); break;
					case O_KEY:
						rotate_tetrimino(well_win_p1, tetrimino_p1, CCW, well_contents_p1); break;
					case ENTER_KEY:
						hold_tetrimino(hold_win_p1, tetrimino_p1, well_contents_p1, &CURRENT_Y_CHECKPOINT,
									  &RECENT_HOLD, &CURRENTLY_HELD_TETRIMINO_ID); break;
				}
				if (ch == KEY_UP) // if tetrimino was dropped
					add_garbage(garbage_win_p1, garbage_win_p2, num_complete_lines_1, args_p1->lock_num,
								&GARBAGE_COUNTER_1, &GARBAGE_COUNTER_2, well_contents_p1);

				draw_well(well_win_p1, tetrimino_p1, well_contents_p1);
				pthread_mutex_unlock(&(tetrimino_lock[args_p1->lock_num]));
				usleep(random() % STALL);
			}

			else if (is_input_useful(ch, controls_p2))
			{
				pthread_mutex_lock(&tetrimino_lock[args_p2->lock_num]);

				switch(ch)
				{
					case A_KEY:
						move_tetrimino(well_win_p2, tetrimino_p2, LEFT, well_contents_p2); break;
					case D_KEY:
						move_tetrimino(well_win_p2, tetrimino_p2, RIGHT, well_contents_p2); break;
					case S_KEY:
						move_tetrimino(well_win_p2, tetrimino_p2, DOWN, well_contents_p2); break;
					case W_KEY:
						num_complete_lines_2 = drop_tetrimino(well_win_p2, tetrimino_p2, args_p2->difficulty, well_contents_p2, 
									  		   &CURRENT_Y_CHECKPOINT_2, &RECENT_HOLD_2); break;
					case G_KEY:
						rotate_tetrimino(well_win_p2, tetrimino_p2, CW, well_contents_p2); break;
					case F_KEY:
						rotate_tetrimino(well_win_p2, tetrimino_p2, CCW, well_contents_p2); break;
					case SPACE_KEY:
						hold_tetrimino(hold_win_p2, tetrimino_p2, well_contents_p2, &CURRENT_Y_CHECKPOINT_2,
									  &RECENT_HOLD_2, &CURRENTLY_HELD_TETRIMINO_ID_2); break;
				}
				if (ch == W_KEY) // if tetrimino was dropped
					add_garbage(garbage_win_p2, garbage_win_p1, num_complete_lines_2, args_p2->lock_num,
								&GARBAGE_COUNTER_2, &GARBAGE_COUNTER_1, well_contents_p2);

				draw_well(well_win_p2, tetrimino_p2, well_contents_p2);
				pthread_mutex_unlock(&(tetrimino_lock[args_p2->lock_num]));
				usleep(random() % STALL);
			}
		}
		else 
		{
			if (state->game_over_flag) break;
			usleep(random() % STALL);
		}
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

	delwin(well_win_p1);
	delwin(well_win_p2);
	delwin(cover_win_p1);
	delwin(cover_win_p2);
	delwin(hold_win_p1);
	delwin(hold_win_p2);
	delwin(garbage_win_p1);
	delwin(garbage_win_p2);
}