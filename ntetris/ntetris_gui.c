#include "ntetris.h"		

								   	


void ntetris_init()
{
	/* Initialize ncurses */
	initscr();

	/* Disable user input from echoing to terminal */
	noecho();

	/* */
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

void print_title(WINDOW *win, char *title[], int title_size)
{
	int num_cols = getmaxx(win);

	int j = 2;
	attron(COLOR_PAIR(TITLE_COLOR_PAIR)); // title is green
	for (int i = 0; i < title_size; i++)
	{
		mvprintw(j, (num_cols - strlen(title[i])) / 2, "%s", title[i]);
		j++;
	}
	attroff(COLOR_PAIR(TITLE_COLOR_PAIR));
}

/* Print the menu and all of its choices, highlighting whichever one
is selected. */

void print_menu(WINDOW *menu_win, int highlight, char *menu_choices[], int num_menu_choices)
{
	int x, y, i;	

	x = 5;
	y = 2;

	for (i = 0; i < num_menu_choices; ++i)
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

int get_menu_choice (char *menu_choices[], int num_menu_choices)
{
	int highlight = START;
	int choice = 0;
	int key_pressed;
	WINDOW *menu_win;

	int MENU_Y = getcury(stdscr) + getmaxy(stdscr) / 4;
	int MENU_X = (getmaxx(stdscr) / 2) - (MENU_WIDTH / 2) ;

	menu_win = newwin(MENU_HEIGHT, MENU_WIDTH, MENU_Y, MENU_X);
	keypad(menu_win, TRUE);

	print_menu(menu_win, highlight, menu_choices, num_menu_choices);
	while(TRUE)
	{	
		key_pressed = wgetch(menu_win);
		switch(key_pressed)
		{	
			case KEY_UP:
				if(highlight == 1)
					highlight = num_menu_choices;
				else
					--highlight;
				break;

			case KEY_DOWN:
				if(highlight == num_menu_choices)
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
		print_menu(menu_win, highlight, menu_choices, num_menu_choices);
		if(choice != 0)	/* User did a choice, so come out of the infinite loop */
			break;
	}

	delwin(menu_win);

	return choice;
}

/* This function draws the well
*/

void draw_well(WINDOW *win, TETRIMINO *tetrimino)
{
	COORDINATE_PAIR shadow_bits[NUM_BITS];
	int i, j;
	int locked_in = 1;
	clear_well(win);

	copy_bits(tetrimino->bits, shadow_bits, NUM_BITS);

	while (valid_position(win, tetrimino, shadow_bits, NUM_BITS)) 
	{
		for (i = 0; i < NUM_BITS; i++)
			shadow_bits[i].y++;

		locked_in = 0;
	}
	
	for (i = 0; i < NUM_BITS; i++)
	{
		if (!locked_in)
			shadow_bits[i].y--;

		mvwaddch(win, shadow_bits[i].y, shadow_bits[i].x, tetrimino->bits[i].value | A_DIM);
	}
	
	for (i = 0; i < NUM_BITS; i++)
		/* Do not draw tetrimino bit if it is currently in the same area as the cover window */
		if (tetrimino->bits[i].y >= COVER_B_BNDRY)
			mvwaddch(win, tetrimino->bits[i].y, tetrimino->bits[i].x, tetrimino->bits[i].value);
		
	for (i = 0; i < WELL_HEIGHT - 2; i++)
		for (j = 0; j < WELL_WIDTH - 2; j++)
			if ((well_contents[i][j].value & A_CHARTEXT) == 'o')
				mvwaddch(win, well_contents[i][j].y, well_contents[i][j].x, well_contents[i][j].value);
				
	wrefresh(win);
}

/* This function "erases" the contents of the well that show up in the terminal
(below the cover window) */

void clear_well(WINDOW *win)
{
	int i, j;

	for (i = WELL_T_BNDRY + 2; i <= WELL_B_BNDRY; i++) 
		for (j = WELL_L_BNDRY; j <= WELL_R_BNDRY; j++)
			mvwaddch(win, i, j, ' ');

	wrefresh(win);
}

int update_hold(WINDOW *win, int tetrimino_id)
{
	int i, j;
	int old_id = CURRENTLY_HELD_TETRIMINO_ID;
	CURRENTLY_HELD_TETRIMINO_ID = tetrimino_id;

	/* First, clear the window contents */
	for (i = HOLD_T_BNDRY; i <= HOLD_B_BNDRY; i++)
		for (j = HOLD_L_BNDRY; j <= HOLD_R_BNDRY; j++)
			mvwaddch(win, i, j, ' ');

	int a, b, c, d;
	int e, f, g, h;

	switch(tetrimino_id)
	{
		case TETRIMINO_I: 
			a = b = c = d = 2;
			e = 2; f = e + 1; g = f + 1; h = g + 1;
			break;
			
		case TETRIMINO_J:
			a = b = c = 2; d = 3; 
			e = 2; f = e + 1; g = f + 1; h = g;
			break;

		case TETRIMINO_L:
			a = b = c = 2; d = 3;
			e = 2; f = e + 1; g = f + 1; h = e;
			break;

		case TETRIMINO_O:
			a = b = 2; c = d = 3;
			e = g = 3; f = h = 4;
			break;

		case TETRIMINO_S:
			a = b = 2; c = d = 3;
			e = h = 3; f = 4; g = 2;
			break;

		case TETRIMINO_T:
			a = b = c = 2; d = 3;
			e = 2; f = e + 1; g = f + 1; h = f;
			break;

		case TETRIMINO_Z:
			a = b = 2; c = d = 3;
			e = 2; f = g = 3; h = 4;
			break; 
	}

	int tetr_y[NUM_BITS] = {a, b, c, d};
	int tetr_x[NUM_BITS] = {e, f, g, h};


	for (i = 0; i < NUM_BITS; i++)
		/* Offset of 3 between ID number and COLOUR_PAIR number*/
		mvwaddch(win, tetr_y[i], tetr_x[i], 'o' | COLOR_PAIR(tetrimino_id + 3));

	wrefresh(win);

	return old_id;
}

void update_line_count(WINDOW *win)
{
	wmove(win, 2, 0);
	wclrtoeol(win);
	mvwprintw(win, 2, 0, "%05d", LINE_COUNT);
	wrefresh(win);
}

void update_level(WINDOW *win)
{
	wmove(win, 2, 0);
	wclrtoeol(win);
	mvwprintw(win, 2, 0, "%03d", LINE_COUNT / 10);
	wrefresh(win);
}

void update_score(WINDOW *win)
{
	wmove(win, 2, 0);
	wclrtoeol(win);
	mvwprintw(win, 2, 0, "%010d", SCORE);
	wrefresh(win);
}

void print_controls()
{
	attron(A_BOLD | COLOR_PAIR(TITLE_COLOR_PAIR));
	mvprintw(4, CONTROLS_INIT_X, "NTETRIS CONTROLS");
	attroff(A_BOLD | COLOR_PAIR(TITLE_COLOR_PAIR));
	mvprintw(6, CONTROLS_INIT_X, "Move tetrimino left                   Left arrow key");
	mvprintw(7, CONTROLS_INIT_X, "Move tetrimino right                  Right arrow key");
	mvprintw(8, CONTROLS_INIT_X, "Move tetrimino down                   Down arrow key");
	mvprintw(10, CONTROLS_INIT_X, "Drop tetrimino                        Up arrow key");
	mvprintw(12, CONTROLS_INIT_X, "Rotate tetrimino clockwise            X");
	mvprintw(13, CONTROLS_INIT_X, "Rotate tetrimino counterclockwise     Z");
	mvprintw(14, CONTROLS_INIT_X, "Hold tetrimino                        Space");

	mvprintw(18, CONTROLS_INIT_X, "Press any key to return");
}

void print_title_small(WINDOW *win)
{
	waddch(win, 'N' | A_BOLD | COLOR_PAIR(I_COLOR_PAIR));
	waddch(win, 'T' | A_BOLD | COLOR_PAIR(J_COLOR_PAIR));
	waddch(win, 'E' | A_BOLD | COLOR_PAIR(L_COLOR_PAIR));
	waddch(win, 'T' | A_BOLD | COLOR_PAIR(O_COLOR_PAIR));
	waddch(win, 'R' | A_BOLD | COLOR_PAIR(S_COLOR_PAIR));
	waddch(win, 'I' | A_BOLD | COLOR_PAIR(T_COLOR_PAIR));
	waddch(win, 'S' | A_BOLD | COLOR_PAIR(Z_COLOR_PAIR));
}