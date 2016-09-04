#include "ntetris.h"

pthread_mutex_t tetrimino_lock[] = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER};
pthread_mutex_t garbage_line_counter_lock[] = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER};

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

/* Thread responsible for moving the tetrimino down one unit at a time with GAME_DELAY pauses. */

void *periodic_thread (void *arguments)
{
	THREAD_ARGS *args = (THREAD_ARGS *) arguments;
	while(TRUE)
	{
		usleep(GAME_DELAY); 
		
		pthread_mutex_lock(&(tetrimino_lock[args->lock_num]));
		move_tetrimino(args->win[WELL_ID], args->tetrimino, DOWN, args->well_contents);

		if (args->mode == SINGLE)
			update_score(args->win[SCORE_ID]);

		draw_well(args->win[WELL_ID], args->tetrimino, args->well_contents);
		pthread_mutex_unlock(&(tetrimino_lock[args->lock_num]));		
	}
}

/* Thread responsible for "locking in" a tetrimino into the well */

void *lock_in_thread (void *arguments)
{
	THREAD_ARGS *args = (THREAD_ARGS *) arguments;

	COORDINATE_PAIR current_bits[NUM_BITS];
	int pivot_bit = args->tetrimino->pivot_bit;
	int num_complete_lines;

	while(TRUE)
	{
		/* Repeatedly check if the tetrimino has fallen past a "checkpoint" that is calculated 
		based on its current position. If it does not fall past the checkpoint within three GAME_DELAYs,
		then it likely cannot fall any further and should be locked in. */

		usleep((GAME_DELAY));
		if (get_y_checkpoint(args->tetrimino->bits) > *(args->current_y_checkpoint))
		{
			*(args->current_y_checkpoint) = get_y_checkpoint(args->tetrimino->bits);
			continue;
		}
		usleep((GAME_DELAY));
		if (get_y_checkpoint(args->tetrimino->bits) > *(args->current_y_checkpoint))
		{
			*(args->current_y_checkpoint) = get_y_checkpoint(args->tetrimino->bits);
			continue;
		}
		usleep((GAME_DELAY));
		if (get_y_checkpoint(args->tetrimino->bits) > *(args->current_y_checkpoint))
		{
			*(args->current_y_checkpoint) = get_y_checkpoint(args->tetrimino->bits);
			continue;
		}

		pthread_mutex_lock(&(tetrimino_lock[args->lock_num]));
		lock_tetrimino_into_well(args->tetrimino, args->well_contents, args->recent_hold);
		num_complete_lines = update_lines(args->win[WELL_ID], args->tetrimino, args->difficulty, args->well_contents);

		if (args->mode == VERSUS)
		{
			add_garbage(args->win[GARBAGE_ID], args->win[OTHER_GARBAGE_ID], num_complete_lines, 
					   args->lock_num, args->garbage_counter, args->other_garbage_counter, args->well_contents);
		}
		if (args->mode == SINGLE)
		{
			update_line_count(args->win[LINE_COUNT_ID]);
			update_score(args->win[SCORE_ID]);
			update_level(args->win[LEVEL_ID]);
		}
		
		init_tetrimino(args->tetrimino, get_rand_num(0, 6), args->well_contents, args->current_y_checkpoint);
		draw_well(args->win[WELL_ID], args->tetrimino, args->well_contents);
		pthread_mutex_unlock(&(tetrimino_lock[args->lock_num]));
	}
}

/* Top-level thread for running single player mode. */

void *play_ntetris_single (void *difficulty) 
{
	/* Allocate memory for tetrimino, necessary windows, and thread arguments */

	WINDOW *well_win = newwin(WELL_HEIGHT, WELL_WIDTH, WELL_INIT_Y, WELL_INIT_X);
	WINDOW *cover_win = newwin(COVER_HEIGHT, COVER_WIDTH, COVER_INIT_Y, COVER_INIT_X);	
	WINDOW *hold_win = newwin(HOLD_HEIGHT, HOLD_WIDTH, HOLD_INIT_Y, HOLD_INIT_X);
	WINDOW *line_count_win = newwin(LINE_COUNT_HEIGHT, LINE_COUNT_WIDTH, LINE_COUNT_INIT_Y, LINE_COUNT_INIT_X);
	WINDOW *score_win = newwin(SCORE_HEIGHT, SCORE_WIDTH, SCORE_INIT_Y, SCORE_INIT_X);
	WINDOW *level_win = newwin(LEVEL_HEIGHT, LEVEL_WIDTH, LEVEL_INIT_Y, LEVEL_INIT_X);
	WINDOW *title_small_win = newwin(TITLE_SMALL_HEIGHT, TITLE_SMALL_WIDTH, TITLE_SMALL_INIT_Y, TITLE_SMALL_INIT_X);
	TETRIMINO *tetrimino = malloc(sizeof(TETRIMINO));
	THREAD_ARGS *args = malloc(sizeof(THREAD_ARGS));

	/* Draw borders for some windows */

	box(well_win, 0, 0);
	wborder(cover_win, ' ', ' ', ' ', 0, ' ', ' ', ACS_ULCORNER, ACS_URCORNER);
	box(hold_win, 0, 0);

	/* Print some bold headings */

	wattron(level_win, A_BOLD);
	wattron(score_win, A_BOLD);
	wattron(line_count_win, A_BOLD);
	mvwprintw(level_win, 0, 0, "Level");
	mvwprintw(score_win, 1, 0, "Score");
	mvwprintw(line_count_win, 0, 0, "Lines Cleared");
	wattroff(level_win, A_BOLD);
	wattroff(score_win, A_BOLD);
	wattroff(line_count_win, A_BOLD);

	update_line_count(line_count_win);
	update_score(score_win);
	update_level(level_win);
	print_title_small(title_small_win);

	wnoutrefresh(well_win);
	wnoutrefresh(cover_win);
	wnoutrefresh(hold_win);
	wnoutrefresh(line_count_win);
	wnoutrefresh(score_win);
	wnoutrefresh(level_win);
	wnoutrefresh(title_small_win);
	doupdate();

	COORDINATE_PAIR well_contents[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH];

	/* Initialize well_contents to be empty */
	int i, j;
	for (i = 0; i < WELL_CONTENTS_HEIGHT; i++)
	{
		for (j = 0; j < WELL_CONTENTS_WIDTH; j++)
		{
			well_contents[i][j].y = i + 1;
			well_contents[i][j].x = j + 1;
			well_contents[i][j].value = ' ';
		}
	}
	
	/* Generate random number seed*/
	srand((unsigned) time(NULL));

	/* Enable input from arrow keys */
	keypad(stdscr, TRUE);

	/* Initialize thread arguments*/
	args->win[WELL_ID] = well_win;
	args->win[COVER_ID] = cover_win;
	args->win[HOLD_ID] = hold_win;
	args->win[LINE_COUNT_ID] = line_count_win;
	args->win[SCORE_ID] = score_win;
	args->win[LEVEL_ID] = level_win;
	args->win[TITLE_SMALL_ID] = title_small_win;
	args->tetrimino = tetrimino;
	args->well_contents = well_contents;
	args->difficulty = *((int *) difficulty);
	args->mode = SINGLE;
	args->lock_num = 0;
	args->current_y_checkpoint = &CURRENT_Y_CHECKPOINT;
	args->recent_hold = &RECENT_HOLD;

	init_tetrimino(tetrimino, get_rand_num(0, 6), well_contents, &CURRENT_Y_CHECKPOINT);
	draw_well(well_win, tetrimino, well_contents);

	pthread_t periodic_t;
	pthread_t lock_in_t;
	
	if (pthread_create(&periodic_t, NULL, &periodic_thread, args))
		printf("Could not run periodic thread\n");
	if (pthread_create(&lock_in_t, NULL, &lock_in_thread, args))
		printf("Could not run lock in thread\n");

	int ch;

	/* Cause getch() calls to wait for user input for 0.1 s (rather than wait indefinitely). 
	If no input is received, getch() returns ERR*/
	halfdelay(1);

	while ((ch = getch()) != QUIT_KEY)
	{
		if (ch != ERR)
		{
			/* Acquire tetrimino lock before we do anything to the tetrimino */
			pthread_mutex_lock(&tetrimino_lock[args->lock_num]);

			switch(ch)
			{
				case KEY_LEFT:
					move_tetrimino(well_win, tetrimino, LEFT, well_contents); break;
				case KEY_RIGHT:
					move_tetrimino(well_win, tetrimino, RIGHT, well_contents); break;
				case KEY_DOWN:
					move_tetrimino(well_win, tetrimino, DOWN, well_contents); break;
				case KEY_UP:
					drop_tetrimino(well_win, tetrimino, args->difficulty, well_contents, 
								  &CURRENT_Y_CHECKPOINT, &RECENT_HOLD); break;
				case P_KEY:
					rotate_tetrimino(well_win, tetrimino, CLOCKWISE, well_contents); break;
				case O_KEY:
					rotate_tetrimino(well_win, tetrimino, CNT_CLOCKWISE, well_contents); break;
				case ENTER_KEY:
					hold_tetrimino(hold_win, tetrimino, well_contents, &CURRENT_Y_CHECKPOINT,
								  &RECENT_HOLD, &CURRENTLY_HELD_TETRIMINO_ID); break;
			}
			
			draw_well(well_win, tetrimino, well_contents);
			update_line_count(line_count_win);
			update_score(score_win);
			update_level(level_win);
			
			/* Done, so release tetrimino lock */
			pthread_mutex_unlock(&(tetrimino_lock[args->lock_num]));

			/* Wait a bit so other threads can acquire lock */
			usleep(random() % STALL);
		}
		else 
		{
			if (GAME_OVER_FLAG) break;
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

	/* Free allocated windows and other structs */
	free(tetrimino);
	free(args);
	delwin(well_win);
	delwin(cover_win);
	delwin(hold_win);
	delwin(line_count_win);
	delwin(score_win);
	delwin(level_win);
	delwin(title_small_win);
}

/* Top-level thread for running versus mode. */

void *play_ntetris_versus (void *unused)
{
	/* Allocate memory for tetrimino, necessary windows, and thread arguments */

	WINDOW *well_win_p1 = newwin(WELL_HEIGHT, WELL_WIDTH, WELL_INIT_Y_P1, WELL_INIT_X_P1);
	WINDOW *well_win_p2 = newwin(WELL_HEIGHT, WELL_WIDTH, WELL_INIT_Y_P2, WELL_INIT_X_P2);
	WINDOW *cover_win_p1 = newwin(COVER_HEIGHT, COVER_WIDTH, COVER_INIT_Y_P1, COVER_INIT_X_P1);
	WINDOW *cover_win_p2 = newwin(COVER_HEIGHT, COVER_WIDTH, COVER_INIT_Y_P2, COVER_INIT_X_P2);
	WINDOW *hold_win_p1 = newwin(HOLD_HEIGHT, HOLD_WIDTH, HOLD_INIT_Y_P1, HOLD_INIT_X_P1);
	WINDOW *hold_win_p2 = newwin(HOLD_HEIGHT, HOLD_WIDTH, HOLD_INIT_Y_P2, HOLD_INIT_X_P2);
	WINDOW *garbage_win_p1 = newwin(GARBAGE_HEIGHT, GARBAGE_WIDTH, GARBAGE_INIT_Y_P1, GARBAGE_INIT_X_P1);
	WINDOW *garbage_win_p2 = newwin(GARBAGE_HEIGHT, GARBAGE_WIDTH, GARBAGE_INIT_Y_P2, GARBAGE_INIT_X_P2);
	TETRIMINO *tetrimino_p1 = malloc(sizeof(TETRIMINO));
	TETRIMINO *tetrimino_p2 = malloc(sizeof(TETRIMINO));
	THREAD_ARGS *args_p1 = malloc(sizeof(THREAD_ARGS));
	THREAD_ARGS *args_p2 = malloc(sizeof(THREAD_ARGS));

	box(well_win_p1, 0, 0);
	box(well_win_p2, 0, 0);
	wborder(cover_win_p1, ' ', ' ', ' ', 0, ' ', ' ', ACS_ULCORNER, ACS_URCORNER);
	wborder(cover_win_p2, ' ', ' ', ' ', 0, ' ', ' ', ACS_ULCORNER, ACS_URCORNER);
	box(hold_win_p1, 0, 0);
	box(hold_win_p2, 0, 0);

	mvwprintw(stdscr, HOLD_INIT_Y_P2 + 10, HOLD_INIT_X_P2, "Player 2");
	mvwprintw(stdscr, HOLD_INIT_Y_P1 + 10, HOLD_INIT_X_P1, "Player 1"); 

	mvwprintw(stdscr, 1, 36, "Press Q");
	mvwprintw(stdscr, 2, 36, "to quit");
	mvwaddch(stdscr, 10, 40, 'N' | A_BOLD | COLOR_PAIR(I_COLOR_PAIR));
	mvwaddch(stdscr, 11, 40,'T' | A_BOLD | COLOR_PAIR(J_COLOR_PAIR));
	mvwaddch(stdscr, 12, 40,'E' | A_BOLD | COLOR_PAIR(L_COLOR_PAIR));
	mvwaddch(stdscr, 13, 40,'T' | A_BOLD | COLOR_PAIR(O_COLOR_PAIR));
	mvwaddch(stdscr, 14, 40,'R' | A_BOLD | COLOR_PAIR(S_COLOR_PAIR));
	mvwaddch(stdscr, 15, 40,'I' | A_BOLD | COLOR_PAIR(T_COLOR_PAIR));
	mvwaddch(stdscr, 16, 40,'S' | A_BOLD | COLOR_PAIR(Z_COLOR_PAIR));
	
	wattron(garbage_win_p1, A_BOLD);
	wattron(garbage_win_p2, A_BOLD);
	mvwprintw(garbage_win_p1, 0, 0, "Incoming");
	mvwprintw(garbage_win_p1, 1, 0, "Garbage");
	mvwprintw(garbage_win_p1, 2, 0, "Lines");
	mvwprintw(garbage_win_p2, 0, 0, "Incoming");
	mvwprintw(garbage_win_p2, 1, 0, "Garbage");
	mvwprintw(garbage_win_p2, 2, 0, "Lines");
	wattroff(garbage_win_p1, A_BOLD);
	wattroff(garbage_win_p2, A_BOLD);
	update_garbage_line_counter(garbage_win_p1, &GARBAGE_COUNTER_1);
	update_garbage_line_counter(garbage_win_p2, &GARBAGE_COUNTER_2);

	wnoutrefresh(stdscr);
	wnoutrefresh(well_win_p1);
	wnoutrefresh(well_win_p2);
	wnoutrefresh(cover_win_p1);
	wnoutrefresh(cover_win_p2);
	wnoutrefresh(hold_win_p1);
	wnoutrefresh(hold_win_p2);
	wnoutrefresh(garbage_win_p1);
	wnoutrefresh(garbage_win_p2);
	doupdate();

	COORDINATE_PAIR well_contents_p1[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH];
	COORDINATE_PAIR well_contents_p2[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH];

	int i, j;
	/* Initialize both well contents to be empty */
	for (i = 0; i < WELL_CONTENTS_HEIGHT; i++)
	{
		for (j = 0; j < WELL_CONTENTS_WIDTH; j++)
		{
			well_contents_p1[i][j].y = i + 1;
			well_contents_p2[i][j].y = i + 1;
			well_contents_p1[i][j].x = j + 1;
			well_contents_p2[i][j].x = j + 1;
			well_contents_p1[i][j].value = ' ';
			well_contents_p2[i][j].value = ' ';
		}
	}

	/* Generate random number seed*/
	srand((unsigned) time(NULL));
	/* Enable input from arrow keys */
	keypad(stdscr, TRUE);

	int controls_p1[NUM_CONTROLS] = {KEY_LEFT, KEY_RIGHT, KEY_DOWN, KEY_UP, P_KEY, O_KEY, ENTER_KEY};
	int controls_p2[NUM_CONTROLS] = {A_KEY, D_KEY, S_KEY, W_KEY, G_KEY, F_KEY, SPACE_KEY};

	/* Initialize thread arguments*/
	args_p1->win[WELL_ID] = well_win_p1;
	args_p1->win[COVER_ID] = cover_win_p1;
	args_p1->win[HOLD_ID] = hold_win_p1;
	args_p1->win[GARBAGE_ID] = garbage_win_p1;
	args_p1->win[OTHER_GARBAGE_ID] = garbage_win_p2;
	args_p1->tetrimino = tetrimino_p1;
	args_p1->well_contents = well_contents_p1;
	args_p1->difficulty = INVALID_DIFF;
	args_p1->mode = VERSUS;
	args_p1->lock_num = 0;
	args_p1->current_y_checkpoint = &CURRENT_Y_CHECKPOINT;
	args_p1->recent_hold = &RECENT_HOLD;
	args_p1->garbage_counter = &GARBAGE_COUNTER_1;
	args_p1->other_garbage_counter = &GARBAGE_COUNTER_2;

	args_p2->win[WELL_ID] = well_win_p2;
	args_p2->win[COVER_ID] = cover_win_p2;
	args_p2->win[HOLD_ID] = hold_win_p2;
	args_p2->win[GARBAGE_ID] = garbage_win_p2;
	args_p2->win[OTHER_GARBAGE_ID] = garbage_win_p1;
	args_p2->tetrimino = tetrimino_p2;
	args_p2->well_contents = well_contents_p2;	
	args_p2->difficulty = INVALID_DIFF;
	args_p2->mode = VERSUS;
	args_p2->lock_num = 1;
	args_p2->current_y_checkpoint = &CURRENT_Y_CHECKPOINT_2;
	args_p2->recent_hold = &RECENT_HOLD_2;
	args_p2->garbage_counter = &GARBAGE_COUNTER_2;
	args_p2->other_garbage_counter = &GARBAGE_COUNTER_1;

	init_tetrimino(tetrimino_p1, get_rand_num(0, 6), well_contents_p1, &CURRENT_Y_CHECKPOINT);
	draw_well(well_win_p1, tetrimino_p1, well_contents_p1);

	init_tetrimino(tetrimino_p2, get_rand_num(0, 6), well_contents_p2, &CURRENT_Y_CHECKPOINT_2);
	draw_well(well_win_p2, tetrimino_p2, well_contents_p2);

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
						rotate_tetrimino(well_win_p1, tetrimino_p1, CLOCKWISE, well_contents_p1); break;
					case O_KEY:
						rotate_tetrimino(well_win_p1, tetrimino_p1, CNT_CLOCKWISE, well_contents_p1); break;
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
						rotate_tetrimino(well_win_p2, tetrimino_p2, CLOCKWISE, well_contents_p2); break;
					case F_KEY:
						rotate_tetrimino(well_win_p2, tetrimino_p2, CNT_CLOCKWISE, well_contents_p2); break;
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
			if (GAME_OVER_FLAG) break;
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

	/* Figure out who won based on an arbitrary flag */
	if (well_contents_p1[0][0].value == 'e')
		WHICH_PLAYER_WON = 2;
	else if (well_contents_p2[0][0].value == 'e')
		WHICH_PLAYER_WON = 1;

	/* Free allocated windows and other structs */
	free(tetrimino_p1);
	free(tetrimino_p2);
	free(args_p1);
	free(args_p2);
	delwin(well_win_p1);
	delwin(well_win_p2);
	delwin(cover_win_p1);
	delwin(cover_win_p2);
	delwin(hold_win_p1);
	delwin(hold_win_p2);
	delwin(garbage_win_p1);
	delwin(garbage_win_p2);
}