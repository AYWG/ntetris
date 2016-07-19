/*
             ______     __       _     
   ____     /_  __/__  / /______(_)____
  / __ \     / / / _ \/ __/ ___/ / ___/
 / / / /    / / /  __/ /_/ /  / (__  ) 
/_/ /_/    /_/  \___/\__/_/  /_/____/  


*/

#include "ntetris.h"

extern int n_menu_choices;

int main(int argc, char **argv)
{
	if (argc > 2)
	{
		printf("Usage: ntetris [--version] [--help]\n");
		exit(1);
	}

	/*
	Initialization.
	*/
	ntetris_init();
	print_title();
	refresh();

	int ch;
	if (get_menu_choice() == START)
	{
		// go to main game
		clear();
		refresh();
		play_ntetris(CASUAL);
		//getch();
	}
	else 
	{
		// exit and return to terminal
	}

	endwin();

	return 0;
}

/* Top-level function for running the game. */

void play_ntetris (int difficulty) 
{
	WINDOW *well_win;
	WINDOW *cover_win;
	
/*	WINDOW *hold_win;
	WINDOW *line_count_win;
	WINDOW *score_win;
*/


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

	box(well_win, 0, 0);
	wborder(cover_win, ' ', ' ', ' ', 0, ' ', ' ', ACS_ULCORNER, ACS_URCORNER);
	
/*	box(hold_win, 0, 0);
	box(line_count_win, 0, 0);
	box(score_win, 0, 0);
*/	

	
	wnoutrefresh(well_win);
	wnoutrefresh(cover_win);
	doupdate();

/*	wrefresh(hold_win);
	wrefresh(line_count_win);
	wrefresh(score_win);
*/


	/* Generate random number seed*/
	
	srand((unsigned) time(NULL));


	int count = 0;
	int ch;

	halfdelay(1);


	while ((ch = wgetch(well_win)) != QUIT_KEY)
	{

		
		if (ch != ERR)
		{
			usleep(50000);
		}
		
		count++;

		if (count == 10)
		{
			// do stuff every second
			printw("one second passed!\n");
			refresh();
			count = 0;
		}

		
	}
	
}