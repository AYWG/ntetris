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

extern int n_menu_choices;

int main()
{
	int highlight = 1;
	int choice = 0;
	int key_pressed;
	WINDOW *menu_win;

	/*
	Initialization.
	*/
	ntetris_init();
	print_title();
	//printw("%d\n", LINES);
	refresh();

	int MENU_Y = getcury(stdscr) + LINES / 4;
	int MENU_X = (getmaxx(stdscr) / 2) - (MENU_WIDTH / 2) ;
	menu_win = newwin(MENU_HEIGHT, MENU_WIDTH, MENU_Y, MENU_X);
	keypad(menu_win, TRUE);
	
	print_menu(menu_win, highlight);
	while(1)
	{	
		key_pressed = wgetch(menu_win);
		switch(key_pressed)
		{	
			case KEY_UP:
				if(highlight == 1)
					highlight = n_menu_choices;
				else
					--highlight;
				break;
			case KEY_DOWN:
				if(highlight == n_menu_choices)
					highlight = 1;
				else 
					++highlight;
				break;
			case ENTER_KEY: 
				choice = highlight;
				break;
			default:
				//mvprintw(24, 0, "Charcter pressed is = %3d Hopefully it can be printed as '%c'", c, c);
				//refresh();
				break;
		}
		print_menu(menu_win, highlight);
		if(choice != 0)	/* User did a choice come out of the infinite loop */
			break;
	}
	/*
	attron(COLOR_PAIR(MAIN_TEXT_COLOR_PAIR));
	printw("Start\n");
	printw("Exit\n");
	attroff(COLOR_PAIR(MAIN_TEXT_COLOR_PAIR));*/
	refresh();
	getch();
	endwin();

	return 0;
}