/* Server code for running ntetris versus mode. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> 
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "ntetris.h"

#define PORT "3490"

#define BACKLOG 2

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void *server_send_thread(void *send_args)
{
	EPlayer i;
	int j, k;
	ClientServerThreadArgs *args = (ClientServerThreadArgs *) send_args;

	int client_socket = args->client_server_socket;
	GameState *state = args->state;

	ServerResponse response;

	while (TRUE)
	{
		response.game_over_flag = state->game_over_flag;
		for (i = PLAYER_1; i < NUM_PLAYERS; i++) {
			response.currently_held_tetrimino[i] = state->currently_held_tetrimino[i];
			response.garbage_line_counter[i] = state->garbage_line[i].counter;

			for (j = 0; j < NUM_BITS; j++) {
				response.tetrimino_bits[i][j] = state->tetrimino[i].bits[j];
				response.tetrimino_bits[i][j] = state->tetrimino[i].bits[j];
			}

			for (j = 0; j < WELL_CONTENTS_HEIGHT; j++)
				for (k = 0; k < WELL_CONTENTS_WIDTH; k++)
					response.well_contents[i][j][k] = state->well_contents[i][j][k];
		}

		if (send(client_socket, &response, sizeof(ServerResponse), MSG_NOSIGNAL) == -1) {
			break;
		}
	}
}


void run_game(int client_socket)
{
	int numbytes, input;
	int num_complete_lines_1;
	int num_complete_lines_2;
	int controls_p1[NUM_CONTROLS] = {KEY_LEFT, KEY_RIGHT, KEY_DOWN, KEY_UP, P_KEY, O_KEY, ENTER_KEY};
	int controls_p2[NUM_CONTROLS] = {A_KEY, D_KEY, S_KEY, W_KEY, G_KEY, F_KEY, SPACE_KEY};

	GameState state;
	
	pthread_t send_t;
	pthread_t periodic_t_p1, periodic_t_p2;
	pthread_t lock_in_t_p1, lock_in_t_p2;
	
	ThreadArgs args_p1;
	args_p1.state = &state;
	args_p1.player_id = PLAYER_1;

	ThreadArgs args_p2;
	args_p2.state = &state;
	args_p2.player_id = PLAYER_2;

	ClientServerThreadArgs args;
	args.client_server_socket = client_socket;
	args.state = &state;

	game_state_init(&state, INVALID_DIFFICULTY, VERSUS);

	// Get well_max dimensions before actually starting
	int well_max[4];
	if (recv(client_socket, well_max, sizeof(int) * 4, 0) == -1) {
		perror("recv well_max");
	}
	state.well_max_x[PLAYER_1] = well_max[0];
	state.well_max_y[PLAYER_1] = well_max[1];
	state.well_max_x[PLAYER_2] = well_max[2];
	state.well_max_y[PLAYER_2] = well_max[3];

	init_tetrimino(&state, PLAYER_1, get_rand_num(0, 6));
	init_tetrimino(&state, PLAYER_2, get_rand_num(0, 6));

	if (pthread_create(&periodic_t_p1, NULL, &periodic_thread, &args_p1))
		printf("Could not run periodic thread\n");
	if (pthread_create(&lock_in_t_p1, NULL, &lock_in_thread, &args_p1))
		printf("Could not run lock in thread\n");

	if (pthread_create(&periodic_t_p2, NULL, &periodic_thread, &args_p2))
		printf("Could not run periodic thread\n");
	if (pthread_create(&lock_in_t_p2, NULL, &lock_in_thread, &args_p2))
		printf("Could not run lock in thread\n");

	if (pthread_create(&send_t, NULL, &server_send_thread, &args))
		printf("Could not run send thread\n");

	while (TRUE) {
		numbytes = recv(client_socket, &input, sizeof(int), 0);
		if (numbytes == -1) {
			perror("server recv");
		}
		if (numbytes <= 0) {
			break;
		}
        printf("server: received '%d'\n", input);

		if (is_input_useful(input, controls_p1)) {
			pthread_mutex_lock(&(state.tetrimino[PLAYER_1].lock));

			switch(input)
			{
				case KEY_LEFT:
					move_tetrimino(&state, PLAYER_1, LEFT); break;
				case KEY_RIGHT:
					move_tetrimino(&state, PLAYER_1, RIGHT); break;
				case KEY_DOWN:
					move_tetrimino(&state, PLAYER_1, DOWN); break;
				case KEY_UP:
					num_complete_lines_1 = drop_tetrimino(&state, PLAYER_1); break;
				case P_KEY:
					rotate_tetrimino(&state, PLAYER_1, CW); break;
				case O_KEY:
					rotate_tetrimino(&state, PLAYER_1, CCW); break;
				case ENTER_KEY:
					hold_tetrimino(&state, PLAYER_1); break;
			}
			if (input == KEY_UP) // if tetrimino was dropped
				add_garbage(&state, PLAYER_2, PLAYER_1, num_complete_lines_1);

			pthread_mutex_unlock(&(state.tetrimino[PLAYER_1].lock));
			usleep(random() % STALL);
		}

		else if (is_input_useful(input, controls_p2))
		{
			pthread_mutex_lock(&(state.tetrimino[PLAYER_2].lock));

			switch(input)
			{
				case A_KEY:
					move_tetrimino(&state, PLAYER_2, LEFT); break;
				case D_KEY:
					move_tetrimino(&state, PLAYER_2, RIGHT); break;
				case S_KEY:
					move_tetrimino(&state, PLAYER_2, DOWN); break;
				case W_KEY:
					num_complete_lines_2 = drop_tetrimino(&state, PLAYER_2); break;
				case G_KEY:
					rotate_tetrimino(&state, PLAYER_2, CW); break;
				case F_KEY:
					rotate_tetrimino(&state, PLAYER_2, CCW); break;
				case SPACE_KEY:
					hold_tetrimino(&state, PLAYER_2); break;
			}
			if (input == W_KEY) // if tetrimino was dropped
				add_garbage(&state, PLAYER_1, PLAYER_2, num_complete_lines_2);

			pthread_mutex_unlock(&(state.tetrimino[PLAYER_2].lock));
			usleep(random() % STALL);
		}
		else {
			usleep(random() % STALL);
		}
		// TODO: how to resolve game over with client
		if (state.game_over_flag) break;
	}

	pthread_cancel(periodic_t_p1);
	pthread_cancel(lock_in_t_p1);
	pthread_cancel(periodic_t_p2);
	pthread_cancel(lock_in_t_p2);

	pthread_cancel(send_t);

	if (pthread_join(periodic_t_p1, NULL))
		printf("Could not properly terminate periodic thread\n");
	if (pthread_join(lock_in_t_p1, NULL))
		printf("Could not properly terminate lock in thread\n");
	if (pthread_join(periodic_t_p2, NULL))
		printf("Could not properly terminate periodic thread\n");
	if (pthread_join(lock_in_t_p2, NULL))
		printf("Could not properly terminate lock in thread\n");

	if (pthread_join(send_t, NULL))
		printf("Could not properly terminate send thread\n");
}

int main (void)
{
	// Networking stuff
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    int new_fd2;
	int numbytes;
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	// struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

    if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

    printf("server: waiting for connections...\n");

	while (TRUE) {
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);

		run_game(new_fd);

		close(new_fd);
	}

    // if (send(new_fd, "Waiting for second player", 25, 0) == -1) {
    //     perror("send");
    // }
    // sin_size = sizeof their_addr;
    // new_fd2 = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    // if (new_fd2 == -1) {
    //     perror("accept");
    // }

    // inet_ntop(their_addr.ss_family,
    //     get_in_addr((struct sockaddr *)&their_addr),
    //     s, sizeof s);
    // printf("server: got connection from %s\n", s);

    // if (send(new_fd, "Both players connected!", 23, 0) == -1) {
    //     perror("send");
    // }

    // if (send(new_fd2, "Both players connected!", 23, 0) == -1) {
    //     perror("send");
    // }

    
    close(sockfd);

    return 0;
}