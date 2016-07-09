/*
             ______     __       _     
   ____     /_  __/__  / /______(_)____
  / __ \     / / / _ \/ __/ ___/ / ___/
 / / / /    / / /  __/ /_/ /  / (__  ) 
/_/ /_/    /_/  \___/\__/_/  /_/____/  


*/

#include <ncurses.h>
//#include <stdlib.h>
#include "ntetris.h"

int main()
{
	/*
	Initialization.
	*/
	ntetris_init();

	
	init_pair(2, COLOR_WHITE, COLOR_BLACK);
	//attron(A_BOLD);
	attron(COLOR_PAIR(Z_COLOR_PAIR));
	printw("Start\n");
	printw("Exit\n");
	//attroff(A_BOLD);
	attroff(COLOR_PAIR(2));
	//box(stdscr, 97, 97);
	refresh();
	getch();
	endwin();

	return 0;
}