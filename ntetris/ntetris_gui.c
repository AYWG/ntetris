#include <ncurses.h>
#include <string.h>
#include "ntetris.h"							 			

/* The title of the game represented in ASCII art */
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

	/* Hide the cursor*/
	curs_set(0);


	start_color();

	/* Define COLOR_ORANGE to have RGB values for orange
	(on scale of 0-1000) */
	init_color(COLOR_ORANGE, 1000, 500, 0);

	/* Initialize color pairs for title, general text, and
	all of the tetris pieces. First color is foreground, 
	second color is background. */
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

/* Displays the title of the game at the top of the screen 
in ASCII. */

void print_title()
{
	int col = getmaxx(stdscr);

	int j = 2;
	attron(COLOR_PAIR(TITLE_COLOR_PAIR)); // title is green
	for (int i = 0; i < 5; i++)
	{
		mvprintw(j, (col - strlen(title[i])) / 2, "%s", title[i]);
		j++;
	}
	attroff(COLOR_PAIR(TITLE_COLOR_PAIR));
}

/* Print the menu and all of its choices, highlighting whichever one
is selected. */

void print_menu(WINDOW *menu_win, int highlight)
{
	int x, y, i;	

	x = 5;
	y = 2;

	for (i = 0; i < n_menu_choices; ++i)
	{	
		if (highlight == i + 1) // Highlight the present choice 
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

/* Initializes the menu window and repeatedly updates the menu 
by calling print_menu. Returns the choice that the user selects. */

int get_menu_choice ()
{
	int highlight = START;
	int choice = 0;
	int key_pressed;
	WINDOW *menu_win;

	int MENU_Y = getcury(stdscr) + LINES / 4;
	int MENU_X = (COLS / 2) - (MENU_WIDTH / 2) ;

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
				break;
		}
		print_menu(menu_win, highlight);
		if(choice != 0)	/* User did a choice come out of the infinite loop */
			break;
	}

	return choice;
}






