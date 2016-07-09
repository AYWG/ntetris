#include <ncurses.h>
#include "ntetris.h"

char *title[] = {"             ______     __       _     ",
				"   ____     /_  __/__  / /______(_)____",
				"  / __ \\     / / / _ \\/ __/ ___/ / ___/",
				" / / / /    / / /  __/ /_/ /  / (__  ) ",
				"/_/ /_/    /_/  \\___/\\__/_/  /_/____/  "};

void ntetris_init()
{
	initscr();

	start_color();
	init_pair(TITLE_COLOR_PAIR, COLOR_GREEN, COLOR_BLACK);
	init_pair(MAIN_TEXT_COLOR_PAIR, COLOR_WHITE, COLOR_BLACK);
	init_pair(I_COLOR_PAIR, COLOR_CYAN, COLOR_BLACK);
	init_pair(J_COLOR_PAIR, COLOR_BLUE, COLOR_BLACK);
	//init_pair(L_COLOR_PAIR, COLOR_, COLOR_BLACK);
	init_pair(O_COLOR_PAIR, COLOR_YELLOW, COLOR_BLACK);
	init_pair(S_COLOR_PAIR, COLOR_GREEN, COLOR_BLACK);
	init_pair(T_COLOR_PAIR, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(Z_COLOR_PAIR, COLOR_RED, COLOR_BLACK);

	attron(COLOR_PAIR(TITLE_COLOR_PAIR));
	for (int i = 0; i < 5; i++)
	{
		printw("%s\n", title[i]);
	}
	attroff(COLOR_PAIR(TITLE_COLOR_PAIR));
}