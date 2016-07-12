/*
             ______     __       _     
   ____     /_  __/__  / /______(_)____
  / __ \     / / / _ \/ __/ ___/ / ___/
 / / / /    / / /  __/ /_/ /  / (__  ) 
/_/ /_/    /_/  \___/\__/_/  /_/____/  


*/

#include <ncurses.h>
#include "ntetris.h"

extern int n_menu_choices;

int main()
{
	/*
	Initialization.
	*/
	ntetris_init();
	print_title();
	refresh();
/*
	if (get_menu_choice() == START)
	{
		// go to main game
		// play_ntetris(difficulty);
	}
	else 
	{
		// exit and return to terminal
	}
*/
	get_menu_choice();
	clear();
	refresh();
	WINDOW *game_win;

	int GAME_Y = 1;
	int GAME_X = COLS / 3;

	game_win = newwin(LINES - 2, GAME_X, GAME_Y, GAME_X);
	//game_win = newwin(LINES, COLS, 0, 0);
	box(game_win, 0, 0);
	wrefresh(game_win);

	getch();
	endwin();

	return 0;
}