/*
             ______     __       _     
   ____     /_  __/__  / /______(_)____
  / __ \     / / / _ \/ __/ ___/ / ___/
 / / / /    / / /  __/ /_/ /  / (__  ) 
/_/ /_/    /_/  \___/\__/_/  /_/____/  


*/

#include "ntetris.h"

pthread_mutex_t tetrimino_lock = PTHREAD_MUTEX_INITIALIZER;

COORDINATE_PAIR well_contents[WELL_HEIGHT - 2][WELL_WIDTH - 2];

int QUIT_FLAG = 0;

int main(int argc, char **argv)
{
	if (argc > 2)
	{
		printf("Usage: ntetris [--version] [--help]\n");
		exit(1);
	}

	pthread_t game_t;

	/*
	Initialization.
	*/
	ntetris_init();
	print_title();
	refresh();

	/* This will later be implemented as an option in a menu */
	int difficulty = CASUAL;

	if (get_menu_choice() == START)
	{
		clear();
		refresh();
		if (pthread_create(&game_t, NULL, &play_ntetris, &difficulty))
			printf("Could not run main phase of game\n");	

		if (pthread_join(game_t, NULL))
			printf("Could not properly terminate main phase of game\n");
	}

	/* Exit ncurses */
	endwin();

	return 0;
}

/* Thread responsible for moving the tetrimino down at the specified fall rate. */

void *periodic_thread(void *arguments)
{
	THREAD_ARGS *args = (THREAD_ARGS *) arguments;
	while(TRUE)
	{
		usleep(args->game_delay >> 1); // change this later
		if (QUIT_FLAG) break;
		usleep(args->game_delay >> 1);
		pthread_mutex_lock(&tetrimino_lock);
		move_tetrimino(args->win, args->tetrimino, KEY_DOWN);
		update_well(args->win, args->tetrimino);
		pthread_mutex_unlock(&tetrimino_lock);
		
		if (QUIT_FLAG) break;
	}
}

/* Thread responsible for "locking in" a tetrimino into the well */

void *lock_in_thread(void *arguments)
{
	THREAD_ARGS *args = (THREAD_ARGS *) arguments;

	COORDINATE_PAIR current_bits[NUM_BITS];

	int i;

	while(TRUE)
	{
		if (QUIT_FLAG) break;

		copy_bits(args->tetrimino->bits, current_bits, NUM_BITS);

		usleep((2 * args->game_delay) >> 2);
		if (!equal_bits(args->tetrimino->bits, current_bits, NUM_BITS))
			continue;
		usleep((2 * args->game_delay) >> 2);
		if (!equal_bits(args->tetrimino->bits, current_bits, NUM_BITS))
			continue;
		usleep((2 * args->game_delay) >> 2);
		if (!equal_bits(args->tetrimino->bits, current_bits, NUM_BITS))
			continue;
		usleep((2 * args->game_delay) >> 2);
		if (!equal_bits(args->tetrimino->bits, current_bits, NUM_BITS))
			continue;

		pthread_mutex_lock(&tetrimino_lock);
		lock_tetrimino_into_well(args->tetrimino);
		init_tetrimino(args->win, args->tetrimino, get_rand_tetrimino());
		update_well(args->win, args->tetrimino);
		pthread_mutex_unlock(&tetrimino_lock);
	}
}

/* Top-level thread for running the game. */

void *play_ntetris (void *difficulty) 
{

	pthread_t periodic_t;
	pthread_t lock_in_t;
	

	WINDOW *well_win;
	WINDOW *cover_win;	
/*	WINDOW *hold_win;
	WINDOW *line_count_win;
	WINDOW *score_win;
*/
	TETRIMINO *tetrimino;

	// set these values in ntetris.h later
/*
	int hold_y = 1;
	int hold_x = COLS / 5;

	int line_count_y = LINES - 4;
	int line_count_x = COLS / 5;

	int score_y = LINES - 4;
	int score_x = 4 * COLS / 5;
*/

	well_win = newwin(WELL_HEIGHT, WELL_WIDTH, WELL_INIT_Y, WELL_INIT_X);
	cover_win = newwin(COVER_HEIGHT, COVER_WIDTH, COVER_INIT_Y, COVER_INIT_X);
/*
	hold_win = newwin(6, 6, hold_y, hold_x);
	line_count_win = newwin(2, 4, line_count_y, line_count_x);
	score_win = newwin(2, 4, score_y, score_x);
*/
	tetrimino = malloc(sizeof(TETRIMINO));

	box(well_win, 0, 0);
	wborder(cover_win, ' ', ' ', ' ', 0, ' ', ' ', ACS_ULCORNER, ACS_URCORNER);
/*	box(hold_win, 0, 0);
	box(line_count_win, 0, 0);
	box(score_win, 0, 0);
*/	

	wnoutrefresh(well_win);
	wnoutrefresh(cover_win);
/*	wnoutrefresh(hold_win);
	wnoutrefresh(line_count_win);
	wnoutrefresh(score_win);
*/
	doupdate();

	/* Initialize well_contents to be empty*/
	int i, j;
	for (i = 0; i < WELL_HEIGHT - 2; i++)
	{
		for (j = 0; j < WELL_WIDTH - 2; j++)
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
	args->win = well_win;
	args->tetrimino = tetrimino;
	args->game_delay = *((int *) difficulty);

	int ch;
	init_tetrimino(well_win, tetrimino, get_rand_tetrimino());
	update_well(well_win, tetrimino);

	
	if (pthread_create(&periodic_t, NULL, &periodic_thread, args))
		printf("Could not run periodic thread\n");
	if (pthread_create(&lock_in_t, NULL, &lock_in_thread, args))
		printf("Could not run lock in thread\n");

	while ((ch = wgetch(well_win)) != QUIT_KEY)
	{
		pthread_mutex_lock(&tetrimino_lock);
		switch(ch)
		{
			case KEY_LEFT:
				move_tetrimino(well_win, tetrimino, KEY_LEFT);
				break;

			case KEY_RIGHT:
				move_tetrimino(well_win, tetrimino, KEY_RIGHT);
				break;

			case KEY_DOWN:
				move_tetrimino(well_win, tetrimino, KEY_DOWN);
				break;

			case KEY_UP:
				drop_tetrimino(well_win, tetrimino);
				break;

			case SPACE_KEY:
				rotate_tetrimino(well_win, tetrimino);
				break;
			/*
			default:
				mvwprintw(well_win, 9, 1, "Invalid key pressed\n");
				wrefresh(well_win); 
				wgetch(well_win);
				break;
			*/				
		}
		update_well(well_win, tetrimino);
		pthread_mutex_unlock(&tetrimino_lock);
		usleep(SMALL_DELAY);
	}

	
	QUIT_FLAG = 1;

	if (pthread_join(periodic_t, NULL))
		printf("Could not properly terminate periodic thread\n");
	if (pthread_join(lock_in_t, NULL))
		printf("Could not properly terminate lock in thread\n");
	
	/* Free allocated windows and other structs */
	
}