/*
Main file for ntetris
*/

#include "ntetris.h"


char *title[] = {
				"          __       __       _     ",
				"   ____  / /____  / /______(_)____",
				"  / __ \\/ __/ _ \\/ __/ ___/ / ___/",
				" / / / / /_/  __/ /_/ /  / (__  ) ",
				"/_/ /_/\\__/\\___/\\__/_/  /_/____/  "
				};

int main(int argc, char **argv)
{
	int STATS = 0;
	if (argc > 2)
	{
		printf("Usage: ntetris [--about] [--help] [--howtoplay] [--stats]\n");
		printf("Example: ntetris --help\n");
		exit(1);
	}

	if (argc == 2)
	{
		if (!strcmp(argv[1],"--about"))
		{
			printf("Current Version: ntetris 1.0.0\n");
			printf("Developed by: Andy Wong\n");
			exit(1);
		}
		else if (!strcmp(argv[1],"--help"))
		{
			print_help_message();
			exit(1);
		}
		else if (!strcmp(argv[1],"--howtoplay"))
		{
			print_howtoplay_message();
			exit(1);
		}
		else if (!strcmp(argv[1],"--stats"))
		{
			STATS = 1;
		}
		else
		{
			printf("Invalid option.\n");
			printf("Usage: ntetris [--about] [--help] [--howtoplay] [--stats]\n");
			printf("Example: ntetris --help\n");
			exit(1);
		}
	}

	GameState state;
	GUI gui;
	pthread_t game_t;
	pthread_t gui_t;

	ntetris_init();
	
	int choice;
	EDifficulty difficulty;
	int row, col;
	getmaxyx(stdscr, row, col);

	char change_term_dim_msg[] = "Please increase terminal dimensions (minimum: 24x80)";

	while (row < 24 || col < 80)
	{
		clear();
		mvprintw(row/2, (col-strlen(change_term_dim_msg))/2, "%s", change_term_dim_msg);
		refresh();
		getmaxyx(stdscr, row, col);
	}

	char *start_menu_choices[] = {
									"Single Player",
									"Versus (2 Players)",
									"Controls", 
									"How To Play",
							 		"Exit"
					   		 	 };				   		 	 

	char *difficulty_menu_choices[] = {
										"Casual",
										"Intermediate",
										"Expert",
										"Back"
								 	  };

	char game_over_msg[] = "GAME OVER";								 	  
	char game_over_msg_opt_1[] = "Press R to return to the main menu";
	char game_over_msg_opt_2[] = "Press any other key to quit";
	char versus_p1_win_msg[] = "Player 1 wins!";
	char versus_p2_win_msg[] = "Player 2 wins!";

	int num_start_menu_choices = sizeof(start_menu_choices) / sizeof (char *);
	int num_diff_menu_choices = sizeof(difficulty_menu_choices) / sizeof (char *);							  	

	while (TRUE)
	{
		clear();
		print_title(stdscr, title, 5);
		refresh();
		if ((choice = get_menu_choice(start_menu_choices, num_start_menu_choices)) == SINGLE)
		{	
			if ((difficulty = get_menu_choice(difficulty_menu_choices, num_diff_menu_choices)) == INVALID_DIFFICULTY)
				continue;
			                                                               
			{
				game_state_init(&state, difficulty, SINGLE);
				clear();
				refresh();
				gui_init(&gui, &state);
				
				if (pthread_create(&gui_t, NULL, &run_gui, &gui))
					printf("Could not run GUI\n");

				if (pthread_create(&game_t, NULL, &play_ntetris_single, &state))
					printf("Could not run main phase of game\n");	

				if (pthread_join(game_t, NULL))
					printf("Could not properly terminate main phase of game\n");
				
				pthread_cancel(gui_t);
				gui_cleanup(&gui);

				if(state.game_over_flag)
				{
					clear();
					attron(COLOR_PAIR(Z_COLOR_PAIR)); // red
					mvprintw(row/2 - 6, (col-strlen(game_over_msg))/2, "%s", game_over_msg);
					attroff(COLOR_PAIR(Z_COLOR_PAIR));
					mvprintw(row/2 - 4, 24, "Final level : %d", state.line_count / 10);
					mvprintw(row/2 - 3, 24, "Final # of lines cleared: %d", state.line_count);
					mvprintw(row/2 - 2, 24, "Final score : %d", state.score);

					mvprintw(row/2 + 2, (col-strlen(game_over_msg_opt_1))/2, "%s", game_over_msg_opt_1);
					mvprintw(row/2 + 3, (col-strlen(game_over_msg_opt_2))/2, "%s", game_over_msg_opt_2);
					
					int game_over_choice = getch();
					if (game_over_choice == RESTART_KEY)
					{
						reset_game_state(&state);
						continue;
					} 
					else break;
				}
				else break;
			}
		}
		else if (choice == VERSUS)
		{
			game_state_init(&state, INVALID_DIFFICULTY, VERSUS);
			clear();
			refresh();
			gui_init(&gui, &state);
			if (pthread_create(&gui_t, NULL, &run_gui, &gui))
				printf("Could not run GUI\n");
			
			if (pthread_create(&game_t, NULL, &play_ntetris_versus, &state))
				printf("Could not run versus phase of game\n");

			if (pthread_join(game_t, NULL))
				printf("Could not properly terminate versus phase of game\n");
			
			pthread_cancel(gui_t);
			gui_cleanup(&gui);

			if(state.game_over_flag)
			{
				clear();
				attron(COLOR_PAIR(Z_COLOR_PAIR)); // red
				if (state.game_over_flag == 1)
					mvprintw(row/2 - 6, (col-strlen(versus_p1_win_msg))/2, "%s", versus_p1_win_msg);
				else if (state.game_over_flag == 2)
					mvprintw(row/2 - 6, (col-strlen(versus_p2_win_msg))/2, "%s", versus_p2_win_msg);
				attroff(COLOR_PAIR(Z_COLOR_PAIR));

				mvprintw(row/2 + 2, (col-strlen(game_over_msg_opt_1))/2, "%s", game_over_msg_opt_1);
				mvprintw(row/2 + 3, (col-strlen(game_over_msg_opt_2))/2, "%s", game_over_msg_opt_2);

				int game_over_choice = getch();
				if (game_over_choice == RESTART_KEY)
				{
					reset_game_state(&state);
					continue;
				} 
				else break;
			}
			else break; 
		}
		else if (choice == CONTROLS)
		{
			clear();
			refresh();
			print_controls();
			getch();
			continue;
		}
		else if (choice == HOWTOPLAY)
		{
			clear();
			refresh();
			print_howtoplay();
			getch();
			continue;
		}
		else break;
	}
	
	/* Exit ncurses */
	endwin();
	
	if(STATS && choice == SINGLE)
	{
		printf("\n----------------------\n");
		printf("Final level : %d\n", state.line_count / 10);
		printf("Final # of lines cleared: %d\n", state.line_count);
		printf("Final score : %d\n", state.score);
	}
	return 0;
}

void print_help_message()
{
	printf("ntetris: a tetris clone that uses the ncurses API.\n");
	printf("Usage: ntetris [--option]\n\n");
	printf("The following options are available:\n\n");
	printf("about\t\tDisplays version and developer information\n");
	printf("help\t\tDisplays this help message\n");
	printf("howtoplay\tDisplays instructions on how to play ntetris\n");
	printf("stats\t\tRuns the game; if player exits during or immediately after\n\t\tlosing a single player game, the player's stats are shown on the \t\tconsole.\n");

}

void print_howtoplay_message()
{
	printf("\n-----------------HOW-TO-PLAY (SINGLE)--------------------------\n\n");
	printf(" - Control the falling tetriminos by moving or rotating them inside the well.\n");
	printf(" - Complete horizontal lines to clear them.\n");
	printf(" - The more lines you clear at once, the more points you earn!\n");
	printf(" - The game ends when the tetriminos stack up to the top of the well.\n\n");

	printf("-----------------HOW-TO-PLAY (VERSUS)-----------------\n\n");
	printf(" - Control the falling tetriminos by moving or rotating them inside your well.\n");
	printf(" - Complete horizontal lines to clear them.\n");
	printf(" - The more lines you clear at once, the more garbage lines you can add to your\n");
	printf("   opponent's well, and the more garbage lines you can take away from your well!\n");
	printf(" - The first player whose tetriminos stack up to the top of their well loses.\n\n");

	printf("-----------------CONTROLS-----------------------------\n\n");
	printf("                                      Single/Player 1      Player 2\n\n");
	printf("Move tetrimino left                   Left arrow key       A\n");
	printf("Move tetrimino right                  Right arrow key      D\n");
	printf("Move tetrimino down                   Down arrow key       S\n");
	printf("Drop tetrimino                        Up arrow key         W\n");
	printf("Rotate tetrimino clockwise            P                    G\n");
	printf("Rotate tetrimino counterclockwise     O                    F\n");
	printf("Hold tetrimino                        Enter                Space\n\n");
}
