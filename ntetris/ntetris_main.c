/*
             ______     __       _     
   ____     /_  __/__  / /______(_)____
  / __ \     / / / _ \/ __/ ___/ / ___/
 / / / /    / / /  __/ /_/ /  / (__  ) 
/_/ /_/    /_/  \___/\__/_/  /_/____/  


*/

#include <ncurses.h>
#include <stdlib.h>
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

	if (get_menu_choice() == START)
	{
		// go to main game
		clear();
		refresh();
		play_ntetris(0);
		getch();
	}
	else 
	{
		// exit and return to terminal
	}

	//getch();
	endwin();

	//return 0;
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
	int well_init_y = 0;
	int well_init_x = COLS / 3;

	int well_height = getmaxy(stdscr) - 1;
	int well_width = well_init_x;
	


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
	//wprintw(well_win, "I'm well_win!");
	//wattron(cover_win, A_BOLD);
	//mvwprintw(well_win, 2, 1, "aaaa");
	//wattroff(cover_win, A_BOLD);

	
	wnoutrefresh(well_win);
	wnoutrefresh(cover_win);
	doupdate();

/*	wrefresh(hold_win);
	wrefresh(line_count_win);
	wrefresh(score_win);
*/




	/* Generate random number seed*/
	
	//srand((unsigned) time(NULL));


/* Algorithm for game flow:
1. Spawn a random tetrimino at the top of the well
	- If this fails, game over; otherwise, continue.
2. Every second, tetrimino falls down one unit. User is able to 
freely move/rotate the tetrimino.
3. Once the piece lands, it becomes "part of the well" in a sense
	- Any complete horizontal lines are cleared (well is updated)
4. A new tetrimino is spawned and the algorithm repeats*/

/*
	while ((ch = getch()) != QUIT_KEY)
	{

	}
*/	
}