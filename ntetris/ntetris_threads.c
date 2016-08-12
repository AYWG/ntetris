#include "ntetris.h"

/* Thread responsible for moving the tetrimino down one unit at a time with GAME_DELAY pauses. */

void *periodic_thread(void *arguments)
{
	THREAD_ARGS *args = (THREAD_ARGS *) arguments;
	while(!GAME_OVER_FLAG)
	{
		usleep(GAME_DELAY / 2); 
		if (GAME_OVER_FLAG) break;
		usleep(GAME_DELAY / 2);
		pthread_mutex_lock(&tetrimino_lock);
		move_tetrimino(args->win[0], args->tetrimino, KEY_DOWN, args->well_contents);
		update_score(args->win[4]);
		draw_well(args->win[0], args->tetrimino, args->well_contents);
		pthread_mutex_unlock(&tetrimino_lock);
		
		if (GAME_OVER_FLAG) break;
	}
}

/* Thread responsible for "locking in" a tetrimino into the well */

void *lock_in_thread(void *arguments)
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

		pthread_mutex_lock(&tetrimino_lock);
		lock_tetrimino_into_well(args->tetrimino, args->well_contents);
		update_lines(args->win[0], args->tetrimino, args->difficulty, args->well_contents);
		update_line_count(args->win[3]);
		update_score(args->win[4]);
		update_level(args->win[5]);
		init_tetrimino(args->tetrimino, get_rand_num(0, 6), args->well_contents);
		draw_well(args->win[0], args->tetrimino, args->well_contents);
		pthread_mutex_unlock(&tetrimino_lock);
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

	THREAD_ARGS *args = malloc(sizeof(THREAD_ARGS));
	args->win[0] = well_win;
	args->win[1] = cover_win;
	args->win[2] = hold_win;
	args->win[3] = line_count_win;
	args->win[4] = score_win;
	args->win[5] = level_win;
	args->win[6] = title_small_win;
	args->tetrimino = tetrimino;
	args->well_contents = well_contents;
	args->difficulty = *((int *) difficulty);

	init_tetrimino(tetrimino, get_rand_num(0, 6), well_contents);
	draw_well(well_win, tetrimino, well_contents);

	pthread_t periodic_t;
	pthread_t lock_in_t;
	
	if (pthread_create(&periodic_t, NULL, &periodic_thread, args))
		printf("Could not run periodic thread\n");
	if (pthread_create(&lock_in_t, NULL, &lock_in_thread, args))
		printf("Could not run lock in thread\n");

	int ch;
	halfdelay(1);
	while ((ch = wgetch(well_win)) != QUIT_KEY)
	{
		if (ch != ERR)
		{
			pthread_mutex_lock(&tetrimino_lock);
			switch(ch)
			{
				case KEY_LEFT:
					move_tetrimino(well_win, tetrimino, KEY_LEFT, well_contents);
					break;

				case KEY_RIGHT:
					move_tetrimino(well_win, tetrimino, KEY_RIGHT, well_contents);
					break;

				case KEY_DOWN:
					move_tetrimino(well_win, tetrimino, KEY_DOWN, well_contents);
					break;

				case KEY_UP:
					drop_tetrimino(well_win, tetrimino, args->difficulty, well_contents);
					break;

				case ROTATE_CW_KEY:
					rotate_tetrimino(well_win, tetrimino, CLOCKWISE, well_contents);
					break;

				case ROTATE_CCW_KEY:
					rotate_tetrimino(well_win, tetrimino, CNT_CLOCKWISE, well_contents);
					break;

				case HOLD_KEY:
					hold_tetrimino(hold_win, tetrimino, well_contents);
					break;		
			}
			draw_well(well_win, tetrimino, well_contents);
			update_line_count(line_count_win);
			update_score(score_win);
			update_level(level_win);
			pthread_mutex_unlock(&tetrimino_lock);
			usleep(SMALL_DELAY);
		}
		else 
		{
			if (GAME_OVER_FLAG) break;
		}
	}
	nocbreak();
	cbreak();

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

void *play_ntetris_versus(void *unused)
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
	getch();

	COORDINATE_PAIR well_contents_p1[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH];
	COORDINATE_PAIR well_contents_p2[WELL_CONTENTS_HEIGHT][WELL_CONTENTS_WIDTH];

}