/* GUI-related functions for ntetris */

#include "ntetris.h"		

static int hold_y[NUM_TETRIMINOS][NUM_BITS] = {{2, 2, 2, 2},
										{2, 2, 2, 3},
										{2, 2, 2, 3},
										{2, 2, 3, 3},
										{2, 2, 3, 3},
										{2, 2, 2, 3},
										{2, 2, 3, 3}
										};

static int hold_x[NUM_TETRIMINOS][NUM_BITS] = {{2, 3, 4, 5},
										{2, 3, 4, 4},
										{2, 3, 4, 2},
										{3, 4, 3, 4},
										{3, 4, 2, 3},
										{2, 3, 4, 3},
										{2, 3, 3, 4} 
										};



/* Initialization function that must be called */
						   
void ntetris_init()
{
	/* Initialize ncurses */
	initscr();

	/* Disable user input from echoing to terminal */
	noecho();

	/* Disable line buffering (characters are available the instant
	they are typed by the user) */
	cbreak();

	/* Hide the cursor*/
	curs_set(0);

	/* Enable colors */
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

/* Initializes main GUI that players see when playing single or versus mode. Assumes that game state has
already been initialized. */
void gui_init(GUI *gui, GameState *state)
{
	gui->state = state;
	gui->refresh_delay = 25000;
	// TODO: init well max y and well max x in state
	
	if (state->mode == SINGLE) {
		// Init windows
		gui->win[PLAYER_1][WELL_ID] = newwin(WELL_HEIGHT, WELL_WIDTH, WELL_INIT_Y, WELL_INIT_X);
		gui->win[PLAYER_1][COVER_ID] = newwin(COVER_HEIGHT, COVER_WIDTH, COVER_INIT_Y, COVER_INIT_X);	
		gui->win[PLAYER_1][HOLD_ID] = newwin(HOLD_HEIGHT, HOLD_WIDTH, HOLD_INIT_Y, HOLD_INIT_X);
		gui->win[PLAYER_1][LINE_COUNT_ID] = newwin(LINE_COUNT_HEIGHT, LINE_COUNT_WIDTH, LINE_COUNT_INIT_Y, LINE_COUNT_INIT_X);
		gui->win[PLAYER_1][SCORE_ID] = newwin(SCORE_HEIGHT, SCORE_WIDTH, SCORE_INIT_Y, SCORE_INIT_X);
		gui->win[PLAYER_1][LEVEL_ID] = newwin(LEVEL_HEIGHT, LEVEL_WIDTH, LEVEL_INIT_Y, LEVEL_INIT_X);
		gui->win[PLAYER_1][TITLE_SMALL_ID] = newwin(TITLE_SMALL_HEIGHT, TITLE_SMALL_WIDTH, TITLE_SMALL_INIT_SINGLE_Y, TITLE_SMALL_INIT_SINGLE_X);

		/* Draw borders for some windows */
		box(gui->win[PLAYER_1][WELL_ID], 0, 0);
		wborder(gui->win[PLAYER_1][COVER_ID], ' ', ' ', ' ', 0, ' ', ' ', ACS_ULCORNER, ACS_URCORNER);
		box(gui->win[PLAYER_1][HOLD_ID], 0, 0);

		/* Print some bold headings */
		wattron(gui->win[PLAYER_1][LEVEL_ID], A_BOLD);
		wattron(gui->win[PLAYER_1][SCORE_ID], A_BOLD);
		wattron(gui->win[PLAYER_1][LINE_COUNT_ID], A_BOLD);
		mvwprintw(gui->win[PLAYER_1][LEVEL_ID], 0, 0, "Level");
		mvwprintw(gui->win[PLAYER_1][SCORE_ID], 1, 0, "Score");
		mvwprintw(gui->win[PLAYER_1][LINE_COUNT_ID], 0, 0, "Lines Cleared");
		wattroff(gui->win[PLAYER_1][LEVEL_ID], A_BOLD);
		wattroff(gui->win[PLAYER_1][SCORE_ID], A_BOLD);
		wattroff(gui->win[PLAYER_1][LINE_COUNT_ID], A_BOLD);

		update_line_count(gui, PLAYER_1);
		update_score(gui, PLAYER_1);
		update_level(gui, PLAYER_1);
		print_title_small(gui);

		wnoutrefresh(gui->win[PLAYER_1][WELL_ID]);
		wnoutrefresh(gui->win[PLAYER_1][COVER_ID]);
		wnoutrefresh(gui->win[PLAYER_1][HOLD_ID]);
		wnoutrefresh(gui->win[PLAYER_1][LINE_COUNT_ID]);
		wnoutrefresh(gui->win[PLAYER_1][SCORE_ID]);
		wnoutrefresh(gui->win[PLAYER_1][LEVEL_ID]);
		wnoutrefresh(gui->win[PLAYER_1][TITLE_SMALL_ID]);
		doupdate();	

		gui->state->well_max_x[PLAYER_1] = getmaxx(gui->win[PLAYER_1][WELL_ID]);
		gui->state->well_max_y[PLAYER_1] = getmaxy(gui->win[PLAYER_1][WELL_ID]);
	}
	
}

void gui_cleanup(GUI *gui)
{
	// Cleanup dependent on single or versus
	delwin(gui->win[PLAYER_1][WELL_ID]);
	delwin(gui->win[PLAYER_1][COVER_ID]);
	delwin(gui->win[PLAYER_1][HOLD_ID]);
	delwin(gui->win[PLAYER_1][LINE_COUNT_ID]);
	delwin(gui->win[PLAYER_1][SCORE_ID]);
	delwin(gui->win[PLAYER_1][LEVEL_ID]);
	delwin(gui->win[PLAYER_1][TITLE_SMALL_ID]);
}

/* Displays the title of the game at the top of the screen 
in ASCII. */

void print_title(WINDOW *win, char *title[], int title_size)
{
	int num_cols = getmaxx(win);

	int i, j = 2;
	attron(COLOR_PAIR(TITLE_COLOR_PAIR)); // title is green

	for (i = 0; i < title_size; i++)
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
		if (highlight == i) // Highlight the present choice 
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
	WINDOW *menu_win;
	int highlight = 0;
	int choice = -1;
	int key_pressed;

	int MENU_HEIGHT = 7;
	int MENU_WIDTH = 23;
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
				if(highlight == 0) // wrap-around from top
					highlight = num_menu_choices - 1;
				else
					--highlight;
				break;

			case KEY_DOWN:
				if(highlight == num_menu_choices - 1) // wrap-around from bottom
					highlight = 0;
				else 
					++highlight;
				break;

			case ENTER_KEY: 
				choice = highlight;
				break;
		}
		print_menu(menu_win, highlight, menu_choices, num_menu_choices);
		if(choice != -1)	/* User made a choice, so come out of loop */
			break;
	}

	delwin(menu_win);

	return choice;
}

/* "Erases" the contents of the well that show up in the terminal
(below the cover window) */

static void clear_well(GUI *gui, EPlayer player_id)
{
	WINDOW *well_win = gui->win[player_id][WELL_ID];
	int i, j;

	for (i = WELL_T_BNDRY + 2; i <= WELL_B_BNDRY; i++) 
		for (j = WELL_L_BNDRY; j <= WELL_R_BNDRY; j++)
			mvwaddch(well_win, i, j, ' ');
}

/* Draw the well, the tetrimino, and the tetrimino's "ghost" which indicates
where it will land in the well */

void update_well(GUI *gui, EPlayer player_id)
{
	WINDOW *win = gui->win[player_id][WELL_ID];
	TETRIMINO *tetrimino = &gui->state->tetrimino[player_id];
	COORDINATE_PAIR shadow_bits[NUM_BITS];
	int i, j;
	int locked_in = 1;
	clear_well(gui, player_id); // erase everything before drawing

	copy_bits(tetrimino->bits, shadow_bits);

	while (valid_position(gui->state, player_id, shadow_bits)) 
	{
		for (i = 0; i < NUM_BITS; i++)
			shadow_bits[i].y++;

		locked_in = 0;
	}
	
	for (i = 0; i < NUM_BITS; i++)
	{
		/* only decrement y coordinates if they changed previously (prev. while loop executed) */
		if (!locked_in) 
			shadow_bits[i].y--;

		/* Only draw shadow bits if they're outside the cover window */
		if (shadow_bits[i].y >= COVER_B_BNDRY)
			mvwaddch(win, shadow_bits[i].y, shadow_bits[i].x, tetrimino->bits[i].value | A_DIM);
	}
	
	for (i = 0; i < NUM_BITS; i++)
		/* Only draw tetrimino bits if they're outside the cover window */
		if (tetrimino->bits[i].y >= COVER_B_BNDRY)
			mvwaddch(win, tetrimino->bits[i].y, tetrimino->bits[i].x, tetrimino->bits[i].value);
		
	for (i = 0; i < WELL_CONTENTS_HEIGHT; i++)
		for (j = 0; j < WELL_CONTENTS_WIDTH; j++)
			/* Only draw well contents if their corresponding character is an 'o' and they are located
			outside the cover window */
			if ((gui->state->well_contents[player_id][i][j].value & A_CHARTEXT) == 'o' && gui->state->well_contents[player_id][i][j].y >= COVER_B_BNDRY)
				mvwaddch(win, gui->state->well_contents[player_id][i][j].y, gui->state->well_contents[player_id][i][j].x, gui->state->well_contents[player_id][i][j].value);
				
	wnoutrefresh(win);
}

/* Updates the player's hold window by displaying the tetrimino specified by
tetrimino_id. */

void update_hold(GUI *gui, EPlayer player_id, int tetrimino_id)
{
	WINDOW *win = gui->win[player_id][HOLD_ID];
	int i, j;

	/* First, clear the window contents */
	for (i = HOLD_T_BNDRY; i <= HOLD_B_BNDRY; i++)
		for (j = HOLD_L_BNDRY; j <= HOLD_R_BNDRY; j++)
			mvwaddch(win, i, j, ' ');

	for (i = 0; i < NUM_BITS; i++)
		/* Offset of 3 between ID number and COLOUR_PAIR number */
		mvwaddch(win, hold_y[tetrimino_id][i], hold_x[tetrimino_id][i], 'o' | COLOR_PAIR(tetrimino_id + 3));

	wnoutrefresh(win);
}

/* Updates the UI with the current number of lines cleared */

void update_line_count(GUI *gui, EPlayer player_id)
{
	WINDOW *line_count_win = gui->win[player_id][LINE_COUNT_ID];
	wmove(line_count_win, 2, 0);
	wclrtoeol(line_count_win);
	mvwprintw(line_count_win, 2, 0, "%05d", gui->state->line_count);
	wnoutrefresh(line_count_win);
}

/* Updates the UI with the current level */

void update_level(GUI *gui, EPlayer player_id)
{
	WINDOW *level_win = gui->win[player_id][LEVEL_ID];
	wmove(level_win, 2, 0);
	wclrtoeol(level_win);
	mvwprintw(level_win, 2, 0, "%03d", gui->state->line_count / 10);
	wnoutrefresh(level_win);
}

/* Updates the UI with the current score */

void update_score(GUI *gui, EPlayer player_id)
{
	WINDOW *score_win = gui->win[player_id][SCORE_ID];
	wmove(score_win, 2, 0);
	wclrtoeol(score_win);
	mvwprintw(score_win, 2, 0, "%010d", gui->state->score);
	wnoutrefresh(score_win);
}

void update_garbage_line_counter(GUI *gui, EPlayer player_id)
{
	WINDOW *garbage_win = gui->win[player_id][GARBAGE_ID];
	wmove(garbage_win, 4, 0);
	wclrtoeol(garbage_win);
	mvwprintw(garbage_win, 4, 0, "%d", gui->state->garbage_counter[player_id]);
	wnoutrefresh(garbage_win);
}

/* Display the controls for playing the game */

void print_controls()
{
	attron(A_BOLD | COLOR_PAIR(TITLE_COLOR_PAIR));
	mvprintw(4, CONTROLS_INIT_X, "NTETRIS CONTROLS");
	attroff(A_BOLD | COLOR_PAIR(TITLE_COLOR_PAIR));
	mvprintw(4, 46, "Single/Player 1");
	mvprintw(4, 67, "Player 2");
	mvprintw(6, CONTROLS_INIT_X, "Move tetrimino left                   Left arrow key       A");
	mvprintw(7, CONTROLS_INIT_X, "Move tetrimino right                  Right arrow key      D");
	mvprintw(8, CONTROLS_INIT_X, "Move tetrimino down                   Down arrow key       S");
	mvprintw(10, CONTROLS_INIT_X, "Drop tetrimino                        Up arrow key         W");
	mvprintw(12, CONTROLS_INIT_X, "Rotate tetrimino clockwise            P                    G");
	mvprintw(13, CONTROLS_INIT_X, "Rotate tetrimino counterclockwise     O                    F");
	mvprintw(14, CONTROLS_INIT_X, "Hold tetrimino                        Enter                Space");
	attron(A_BOLD);
	mvprintw(18, CONTROLS_INIT_X, "Press any key to return");
	attroff(A_BOLD);
}

/* Displays instructions for how to play the game */

void print_howtoplay()
{
	attron(A_BOLD | COLOR_PAIR(TITLE_COLOR_PAIR));
	mvprintw(2, HOWTOPLAY_INIT_X, "HOW TO PLAY NTETRIS (SINGLE)");
	attroff(A_BOLD | COLOR_PAIR(TITLE_COLOR_PAIR));
	mvprintw(4, HOWTOPLAY_INIT_X, "- Control the falling tetriminos by moving or rotating them inside");
	mvprintw(5, HOWTOPLAY_INIT_X, "  the well.");
	mvprintw(6, HOWTOPLAY_INIT_X, "- Complete horizontal lines to clear them.");
	mvprintw(7, HOWTOPLAY_INIT_X, "- The more lines you clear at once, the more points you earn!");
	mvprintw(8, HOWTOPLAY_INIT_X, "- The game ends when the tetriminos stack up to the top of the well.");

	attron(A_BOLD | COLOR_PAIR(TITLE_COLOR_PAIR));
	mvprintw(10, HOWTOPLAY_INIT_X, "HOW TO PLAY NTETRIS (VERSUS)");
	attroff(A_BOLD | COLOR_PAIR(TITLE_COLOR_PAIR));
	mvprintw(12, HOWTOPLAY_INIT_X, "- Control the falling tetriminos by moving or rotating them inside");
	mvprintw(13, HOWTOPLAY_INIT_X, "  your well.");
	mvprintw(14, HOWTOPLAY_INIT_X, "- Complete horizontal lines to clear them.");
	mvprintw(15, HOWTOPLAY_INIT_X, "- The more lines you clear at once, the more garbage lines you can");
	mvprintw(16, HOWTOPLAY_INIT_X, "  add to your opponent's well, and the more garbage lines you can");
	mvprintw(17, HOWTOPLAY_INIT_X, "  take away from your well!");
	mvprintw(18, HOWTOPLAY_INIT_X, "- The first player whose tetriminos stack up to the top of their");
	mvprintw(19, HOWTOPLAY_INIT_X, "  well loses.");
	attron(A_BOLD);
	mvprintw(21, HOWTOPLAY_INIT_X, "Press any key to return");
	attroff(A_BOLD);
}

/* Prints a small version of the title in the UI */

void print_title_small(GUI *gui)
{
	WINDOW *win = gui->win[PLAYER_1][TITLE_SMALL_ID];
	mvwaddch(win, 0, 0, 'N' | A_BOLD | COLOR_PAIR(I_COLOR_PAIR));
	mvwaddch(win, 1, 0,'T' | A_BOLD | COLOR_PAIR(J_COLOR_PAIR));
	mvwaddch(win, 2, 0,'E' | A_BOLD | COLOR_PAIR(L_COLOR_PAIR));
	mvwaddch(win, 3, 0,'T' | A_BOLD | COLOR_PAIR(O_COLOR_PAIR));
	mvwaddch(win, 4, 0,'R' | A_BOLD | COLOR_PAIR(S_COLOR_PAIR));
	mvwaddch(win, 5, 0,'I' | A_BOLD | COLOR_PAIR(T_COLOR_PAIR));
	mvwaddch(win, 6, 0,'S' | A_BOLD | COLOR_PAIR(Z_COLOR_PAIR));

	mvwprintw(win, 9, 0, "Press Q to quit");
	wnoutrefresh(win);
}