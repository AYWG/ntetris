#include "ntetris.h"

pthread_mutex_t tetrimino_lock[] = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER};

/* Thread responsible for getting input from the user */

void *get_user_input_thread (void *arguments)
{
	THREAD_ARGS *args = (THREAD_ARGS *) arguments;

	int ch;
	halfdelay(1);

	while ((ch = wgetch(args->win[WELL_ID])) != QUIT_KEY)
	{
		if (ch != ERR)
		{
			pthread_mutex_lock(&tetrimino_lock[args->lock_num]);

			if (ch == args->controls[MOVE_LEFT])
				move_tetrimino(args->win[WELL_ID], args->tetrimino, LEFT, args->well_contents);
			else if (ch == args->controls[MOVE_RIGHT])
				move_tetrimino(args->win[WELL_ID], args->tetrimino, RIGHT, args->well_contents);
			else if (ch == args->controls[MOVE_DOWN])
				move_tetrimino(args->win[WELL_ID], args->tetrimino, DOWN, args->well_contents);
			else if (ch == args->controls[DROP])
				drop_tetrimino(args->win[WELL_ID], args->tetrimino, args->difficulty, args->well_contents);
			else if (ch == args->controls[ROTATE_CW])
				rotate_tetrimino(args->win[WELL_ID], args->tetrimino, CLOCKWISE, args->well_contents);
			else if (ch == args->controls[ROTATE_CCW])
				rotate_tetrimino(args->win[WELL_ID], args->tetrimino, CNT_CLOCKWISE, args->well_contents);
			else if (ch == args->controls[HOLD])
				hold_tetrimino(args->win[HOLD_ID], args->tetrimino, args->well_contents);
			
			draw_well(args->win[WELL_ID], args->tetrimino, args->well_contents);
			if (args->mode == SINGLE)
			{
				update_line_count(args->win[LINE_COUNT_ID]);
				update_score(args->win[SCORE_ID]);
				update_level(args->win[LEVEL_ID]);
			}
			pthread_mutex_unlock(&(tetrimino_lock[args->lock_num]));
			usleep(SMALL_DELAY);
		}
		else 
		{
			if (GAME_OVER_FLAG) break;
		}
	}

	nocbreak();
	cbreak();
}

/* Thread responsible for moving the tetrimino down one unit at a time with GAME_DELAY pauses. */

void *periodic_thread (void *arguments)
{
	THREAD_ARGS *args = (THREAD_ARGS *) arguments;
	while(!GAME_OVER_FLAG)
	{
		usleep(GAME_DELAY / 2); 
		if (GAME_OVER_FLAG) break;
		usleep(GAME_DELAY / 2);

		pthread_mutex_lock(&(tetrimino_lock[args->lock_num]));
		move_tetrimino(args->win[WELL_ID], args->tetrimino, DOWN, args->well_contents);
		if (args->mode == SINGLE)
			update_score(args->win[SCORE_ID]);
		draw_well(args->win[WELL_ID], args->tetrimino, args->well_contents);
		pthread_mutex_unlock(&(tetrimino_lock[args->lock_num]));
		
		if (GAME_OVER_FLAG) break;
	}
}

/* Thread responsible for "locking in" a tetrimino into the well */

void *lock_in_thread (void *arguments)
{
	THREAD_ARGS *args = (THREAD_ARGS *) arguments;

	COORDINATE_PAIR current_bits[NUM_BITS];

	while(!GAME_OVER_FLAG)
	{
		if (GAME_OVER_FLAG) break;

		copy_bits(args->tetrimino->bits, current_bits);

		usleep((GAME_DELAY) / 2);
		if (!equal_bits(args->tetrimino->bits, current_bits))
			continue;
		usleep((GAME_DELAY) / 2);
		if (!equal_bits(args->tetrimino->bits, current_bits))
			continue;
		usleep((GAME_DELAY) / 2);
		if (!equal_bits(args->tetrimino->bits, current_bits))
			continue;
		usleep((GAME_DELAY) / 2);
		if (!equal_bits(args->tetrimino->bits, current_bits))
			continue;

		pthread_mutex_lock(&(tetrimino_lock[args->lock_num]));
		lock_tetrimino_into_well(args->tetrimino, args->well_contents);
		update_lines(args->win[WELL_ID], args->tetrimino, args->difficulty, args->well_contents);
		update_line_count(args->win[LINE_COUNT_ID]);
		update_score(args->win[SCORE_ID]);
		update_level(args->win[LEVEL_ID]);
		init_tetrimino(args->tetrimino, get_rand_num(0, 6), args->well_contents);
		draw_well(args->win[WELL_ID], args->tetrimino, args->well_contents);
		pthread_mutex_unlock(&(tetrimino_lock[args->lock_num]));
	}
}

/* Top-level thread for running the game. */

void *play_ntetris_single (void *difficulty) 
{
	/* Allocate memory for tetrimino and necessary windows */

	WINDOW *well_win = newwin(WELL_HEIGHT, WELL_WIDTH, WELL_INIT_Y, WELL_INIT_X);
	WINDOW *cover_win = newwin(COVER_HEIGHT, COVER_WIDTH, COVER_INIT_Y, COVER_INIT_X);	
	WINDOW *hold_win = newwin(HOLD_HEIGHT, HOLD_WIDTH, HOLD_INIT_Y, HOLD_INIT_X);
	WINDOW *line_count_win = newwin(LINE_COUNT_HEIGHT, LINE_COUNT_WIDTH, LINE_COUNT_INIT_Y, LINE_COUNT_INIT_X);
	WINDOW *score_win = newwin(SCORE_HEIGHT, SCORE_WIDTH, SCORE_INIT_Y, SCORE_INIT_X);
	WINDOW *level_win = newwin(LEVEL_HEIGHT, LEVEL_WIDTH, LEVEL_INIT_Y, LEVEL_INIT_X);
	WINDOW *title_small_win = newwin(TITLE_SMALL_HEIGHT, TITLE_SMALL_WIDTH, TITLE_SMALL_INIT_Y, TITLE_SMALL_INIT_X);
	TETRIMINO *tetrimino = malloc(sizeof(TETRIMINO));

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
	keypad(well_win, TRUE);

	int controls[NUM_CONTROLS] = {KEY_LEFT, KEY_RIGHT, KEY_DOWN, KEY_UP, P_KEY, O_KEY, ENTER_KEY};

	THREAD_ARGS *args = malloc(sizeof(THREAD_ARGS));
	args->win[WELL_ID] = well_win;
	args->win[COVER_ID] = cover_win;
	args->win[HOLD_ID] = hold_win;
	args->win[LINE_COUNT_ID] = line_count_win;
	args->win[SCORE_ID] = score_win;
	args->win[LEVEL_ID] = level_win;
	args->win[TITLE_SMALL_ID] = title_small_win;
	args->tetrimino = tetrimino;
	args->well_contents = well_contents;

	for (i = 0; i < NUM_CONTROLS; i++)
		args->controls[i] = controls[i];

	args->difficulty = *((int *) difficulty);
	args->mode = SINGLE;
	args->lock_num = 0;

	init_tetrimino(tetrimino, get_rand_num(0, 6), well_contents);
	draw_well(well_win, tetrimino, well_contents);

	pthread_t periodic_t;
	pthread_t lock_in_t;
	pthread_t get_user_input_t;
	
	if (pthread_create(&periodic_t, NULL, &periodic_thread, args))
		printf("Could not run periodic thread\n");
	if (pthread_create(&lock_in_t, NULL, &lock_in_thread, args))
		printf("Could not run lock in thread\n");
	if (pthread_create(&get_user_input_t, NULL, &get_user_input_thread, args))
		printf("Could not run lock in thread\n");
	
	if (pthread_join(get_user_input_t, NULL))
		printf("Could not properly terminate user input thread\n");
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

void *play_ntetris_versus (void *unused)
{
	WINDOW *well_win_p1 = newwin(WELL_HEIGHT, WELL_WIDTH, WELL_INIT_Y_P1, WELL_INIT_X_P1);
	WINDOW *well_win_p2 = newwin(WELL_HEIGHT, WELL_WIDTH, WELL_INIT_Y_P2, WELL_INIT_X_P2);
	WINDOW *cover_win_p1 = newwin(COVER_HEIGHT, COVER_WIDTH, COVER_INIT_Y_P1, COVER_INIT_X_P1);
	WINDOW *cover_win_p2 = newwin(COVER_HEIGHT, COVER_WIDTH, COVER_INIT_Y_P2, COVER_INIT_X_P2);
	WINDOW *hold_win_p1 = newwin(HOLD_HEIGHT, HOLD_WIDTH, HOLD_INIT_Y_P1, HOLD_INIT_X_P1);
	WINDOW *hold_win_p2 = newwin(HOLD_HEIGHT, HOLD_WIDTH, HOLD_INIT_Y_P2, HOLD_INIT_X_P2);
	TETRIMINO *tetrimino_p1 = malloc(sizeof(TETRIMINO));
	TETRIMINO *tetrimino_p2 = malloc(sizeof(TETRIMINO));

	box(well_win_p1, 0, 0);
	box(well_win_p2, 0, 0);
	wborder(cover_win_p1, ' ', ' ', ' ', 0, ' ', ' ', ACS_ULCORNER, ACS_URCORNER);
	wborder(cover_win_p2, ' ', ' ', ' ', 0, ' ', ' ', ACS_ULCORNER, ACS_URCORNER);
	box(hold_win_p1, 0, 0);
	box(hold_win_p2, 0, 0);

	wnoutrefresh(well_win_p1);
	wnoutrefresh(well_win_p2);
	wnoutrefresh(cover_win_p1);
	wnoutrefresh(cover_win_p2);
	wnoutrefresh(hold_win_p1);
	wnoutrefresh(hold_win_p2);
	doupdate();

	COORDINATE_PAIR well_contents_p1[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH];
	COORDINATE_PAIR well_contents_p2[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH];

	int i, j;
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

	srand((unsigned) time(NULL));

	keypad(well_win_p1, TRUE);
	keypad(well_win_p2, TRUE);

	THREAD_ARGS *args_p1 = malloc(sizeof(THREAD_ARGS));
	THREAD_ARGS *args_p2 = malloc(sizeof(THREAD_ARGS));

	int controls_p1[NUM_CONTROLS] = {KEY_LEFT, KEY_RIGHT, KEY_DOWN, KEY_UP, P_KEY, O_KEY, ENTER_KEY};
	int controls_p2[NUM_CONTROLS] = {A_KEY, D_KEY, S_KEY, W_KEY, G_KEY, F_KEY, SPACE_KEY};

	args_p1->win[WELL_ID] = well_win_p1;
	args_p1->win[COVER_ID] = cover_win_p1;
	args_p1->win[HOLD_ID] = hold_win_p1;
	args_p1->tetrimino = tetrimino_p1;
	args_p1->well_contents = well_contents_p1;

	args_p2->win[WELL_ID] = well_win_p2;
	args_p2->win[COVER_ID] = cover_win_p2;
	args_p2->win[HOLD_ID] = hold_win_p2;
	args_p2->tetrimino = tetrimino_p2;
	args_p2->well_contents = well_contents_p2;

	for (i = 0; i < NUM_CONTROLS; i++)
	{
		args_p1->controls[i] = controls_p1[i];
		args_p2->controls[i] = controls_p2[i];
	}

	args_p1->mode = VERSUS;
	args_p2->mode = VERSUS;
	args_p1->lock_num = 0;
	args_p2->lock_num = 1;

	init_tetrimino(tetrimino_p1, get_rand_num(0, 6), well_contents_p1);
	draw_well(well_win_p1, tetrimino_p1, well_contents_p1);

	init_tetrimino(tetrimino_p2, get_rand_num(0, 6), well_contents_p2);
	draw_well(well_win_p2, tetrimino_p2, well_contents_p2);

	
	pthread_t periodic_t_p1, periodic_t_p2;
	pthread_t lock_in_t_p1, lock_in_t_p2;
	pthread_t get_user_input_t_p1, get_user_input_t_p2;

	if (pthread_create(&periodic_t_p1, NULL, &periodic_thread, args_p1))
		printf("Could not run periodic thread\n");
	if (pthread_create(&lock_in_t_p1, NULL, &lock_in_thread, args_p1))
		printf("Could not run lock in thread\n");
	if (pthread_create(&get_user_input_t_p1, NULL, &get_user_input_thread, args_p1))
		printf("Could not run lock in thread\n");

	if (pthread_create(&periodic_t_p2, NULL, &periodic_thread, args_p2))
		printf("Could not run periodic thread\n");
	if (pthread_create(&lock_in_t_p2, NULL, &lock_in_thread, args_p2))
		printf("Could not run lock in thread\n");
	if (pthread_create(&get_user_input_t_p2, NULL, &get_user_input_thread, args_p2))
		printf("Could not run lock in thread\n");
	
	if (pthread_join(get_user_input_t_p1, NULL))
		printf("Could not properly terminate user input thread\n");
	if (pthread_join(get_user_input_t_p2, NULL))
		printf("Could not properly terminate user input thread\n");

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