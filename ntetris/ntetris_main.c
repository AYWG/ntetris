/*
             ______     __       _     
   ____     /_  __/__  / /______(_)____
  / __ \     / / / _ \/ __/ ___/ / ___/
 / / / /    / / /  __/ /_/ /  / (__  ) 
/_/ /_/    /_/  \___/\__/_/  /_/____/  


*/

#include "ntetris.h"

pthread_mutex_t tetrimino_lock = PTHREAD_MUTEX_INITIALIZER;

int GAME_DELAY;
int GAME_OVER_FLAG = 0;
int RECENT_HOLD = 0;
int CURRENTLY_HELD_TETRIMINO_ID = INVALID_ID;
int LINE_COUNT = 0;
int SCORE = 0;

char *title[] = {
				"             ______     __       _     ",
				"   ____     /_  __/__  / /______(_)____",
				"  / __ \\     / / / _ \\/ __/ ___/ / ___/",
				" / / / /    / / /  __/ /_/ /  / (__  ) ",
				"/_/ /_/    /_/  \\___/\\__/_/  /_/____/  "
				};

int main(int argc, char **argv)
{
	int STATS = 0;
	if (argc > 2)
	{
		printf("Usage: ntetris [--version] [--help] [--stats]\n");
		exit(1);
	}

	if (argc == 2)
	{
		if (!strcmp(argv[1],"--version"))
		{
			printf("Current Version: ntetris 1.0.0\n");
			exit(1);
		}

		if (!strcmp(argv[1],"--help"))
		{
			print_help_message();
			exit(1);
		}

		if (!strcmp(argv[1],"--stats"))
		{
			STATS = 1;
		}
	}

	pthread_t game_t;

	ntetris_init();

	
	int choice;
	int difficulty;
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

	int num_start_menu_choices = sizeof(start_menu_choices) / sizeof (char *);
	int num_diff_menu_choices = sizeof(difficulty_menu_choices) / sizeof (char *);							  	

	while (TRUE)
	{
		clear();
		print_title(stdscr, title, 5);
		refresh();
		if ((choice = get_menu_choice(start_menu_choices, num_start_menu_choices)) == SINGLE)
		{	
			if ((difficulty = get_menu_choice(difficulty_menu_choices, num_diff_menu_choices)) == BACK)
				continue;
			else
			{
				switch(difficulty)
				{
					case CASUAL: GAME_DELAY = CASUAL_INIT_DELAY; break;
					case INTERMEDIATE: GAME_DELAY = INTERMEDIATE_INIT_DELAY; break;
					case EXPERT: GAME_DELAY = EXPERT_INIT_DELAY; break;
				}
				clear();
				refresh();
				if (pthread_create(&game_t, NULL, &play_ntetris_single, &difficulty))
					printf("Could not run main phase of game\n");	

				if (pthread_join(game_t, NULL))
					printf("Could not properly terminate main phase of game\n");
				
				if(GAME_OVER_FLAG)
				{
					clear();
					attron(COLOR_PAIR(Z_COLOR_PAIR)); // red
					mvprintw(row/2 - 6, (col-strlen(game_over_msg))/2, "%s", game_over_msg);
					attroff(COLOR_PAIR(Z_COLOR_PAIR));
					mvprintw(row/2 - 4, 24, "Final level : %d", LINE_COUNT / 10);
					mvprintw(row/2 - 3, 24, "Final # of lines cleared: %d", LINE_COUNT);
					mvprintw(row/2 - 2, 24, "Final score : %d", SCORE);

					mvprintw(row/2 + 2, (col-strlen(game_over_msg_opt_1))/2, "%s", game_over_msg_opt_1);
					mvprintw(row/2 + 3, (col-strlen(game_over_msg_opt_2))/2, "%s", game_over_msg_opt_2);
					
					int game_over_choice = getch();
					if (game_over_choice == RESTART_KEY) continue;
					else break;
				}
				else break;
			}
		}
		else if (choice == VERSUS)
		{
			GAME_DELAY = CASUAL_INIT_DELAY;
			// spawn versus thread
			clear();
			refresh();
			if (pthread_create(&game_t, NULL, &play_ntetris_versus, NULL))
				printf("Could not run versus phase of game\n");

			if (pthread_join(game_t, NULL))
				printf("Could not properly terminate versus phase of game\n");
			break; 
		}
		else if (choice == CONTROLS)
		{
			clear();
			refresh();
			print_controls();
			getch();
			continue;
		}
		else break;
	}
	
	/* Exit ncurses */
	endwin();
	
	if(STATS)
	{
		printf("\n----------------------\n");
		printf("Final level : %d\n", LINE_COUNT / 10);
		printf("Final # of lines cleared: %d\n", LINE_COUNT);
		printf("Final score : %d\n", SCORE);
	}
	return 0;
}

void print_help_message()
{
	printf("ntetris: a tetris clone that uses the ncurses API\n");
}

