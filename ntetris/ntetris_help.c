#include <ncurses.h>
#include <string.h>
#include "ntetris.h"

char *title[] = {
					"             ______     __       _     ",
					"   ____     /_  __/__  / /______(_)____",
					"  / __ \\     / / / _ \\/ __/ ___/ / ___/",
					" / / / /    / / /  __/ /_/ /  / (__  ) ",
					"/_/ /_/    /_/  \\___/\\__/_/  /_/____/  "
				};

char *menu_choices[] = {
							"Start",
							 "Exit"
					   };	

int n_menu_choices = sizeof(menu_choices) / sizeof(char *);							 			

void ntetris_init()
{
	initscr();
	noecho();
	cbreak();

	start_color();
	init_color(COLOR_ORANGE, 1000, 500, 0);
	init_pair(TITLE_COLOR_PAIR, COLOR_GREEN, COLOR_BLACK);
	init_pair(MAIN_TEXT_COLOR_PAIR, COLOR_WHITE, COLOR_BLACK);
	init_pair(I_COLOR_PAIR, COLOR_CYAN, COLOR_BLACK);
	init_pair(J_COLOR_PAIR, COLOR_BLUE, COLOR_BLACK);
	init_pair(L_COLOR_PAIR, COLOR_ORANGE, COLOR_BLACK);
	init_pair(O_COLOR_PAIR, COLOR_YELLOW, COLOR_BLACK);
	init_pair(S_COLOR_PAIR, COLOR_GREEN, COLOR_BLACK);
	init_pair(T_COLOR_PAIR, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(Z_COLOR_PAIR, COLOR_RED, COLOR_BLACK);
}

/**/

void print_title()
{
	int col = getmaxx(stdscr);

	int j = 2;
	attron(COLOR_PAIR(TITLE_COLOR_PAIR));
	for (int i = 0; i < 5; i++)
	{
		mvprintw(j, (col - strlen(title[i])) / 2, "%s", title[i]);
		j++;
	}
	attroff(COLOR_PAIR(TITLE_COLOR_PAIR));
}

/**/

void print_menu(WINDOW *menu_win, int highlight)
{
	int x, y, i;	

	x = 5;
	y = 2;
	//box(menu_win, 0, 0);
	for(i = 0; i < n_menu_choices; ++i)
	{	if(highlight == i + 1) /* High light the present choice */
		{	wattron(menu_win, A_REVERSE); 
			mvwprintw(menu_win, y, x, "%s", menu_choices[i]);
			wattroff(menu_win, A_REVERSE);
		}
		else
			mvwprintw(menu_win, y, x, "%s", menu_choices[i]);
		++y;
	}
	wrefresh(menu_win);
}


