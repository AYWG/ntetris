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

	if (get_menu_choice() == START)
	{
		// go to main game
		clear();
		refresh();
		play_ntetris();
		getch();
	}
	else 
	{
		// exit and return to terminal
	}

	//getch();
	endwin();

	return 0;
}