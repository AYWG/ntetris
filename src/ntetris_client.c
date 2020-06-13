/* Networking code for clients in versus mode */

#include "ntetris.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> 
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>

#define PORT "3490"

#define MAXDATASIZE 100

// get sockaddr, IPv4 or IPv6:
static void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

static int connect_to_server(const char * hostname) {
    int sockfd, numbytes;  
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(hostname, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
	}

	freeaddrinfo(servinfo); // all done with this structure
	return sockfd;
}

static void *client_recv_thread(void *recv_args)
{
	EPlayer i;
	int numbytes, j, k;
	ClientThreadArgs* args = (ClientThreadArgs *) recv_args;

	int socket = args->server_socket;
	GameState *state = args->state;
	ServerResponse response;

	while (TRUE)
	{
		if ((numbytes = recv(socket, &response, sizeof(ServerResponse), MSG_WAITALL)) == -1) {
			perror("recv");
		}
		state->game_over_flag = response.game_over_flag;
		for (i = PLAYER_1; i < NUM_PLAYERS; i++) {
			state->currently_held_tetrimino[i] = response.currently_held_tetrimino[i];
			state->garbage_line[i].counter = response.garbage_line_counter[i];

			for (j = 0; j < NUM_BITS; j++) {
				state->tetrimino[i].bits[j] = response.tetrimino_bits[i][j];
				state->tetrimino[i].bits[j] = response.tetrimino_bits[i][j];
			}

			for (j = 0; j < WELL_CONTENTS_HEIGHT; j++)
				for (k = 0; k < WELL_CONTENTS_WIDTH; k++)
					state->well_contents[i][j][k] = response.well_contents[i][j][k];
		}
	}

}

static void get_countdown(int server_socket, EPlayer player_id)
{
	int countdown_val; 
	do {
		if (recv(server_socket, &countdown_val, sizeof(int), 0) == -1) {
			perror("recv countdown");
		}

		print_countdown(countdown_val, player_id);

	} while (countdown_val > 0);
	
}

EGameOver play_ntetris_remote() {
	GameState state;
	GUI gui;
	pthread_t gui_t, recv_t;
	EPlayer player_id;
	ClientThreadArgs args;
	EGameOver game_over_status = NOT_OVER;
	int ch, server_socket;
	char waiting_msg[] = "Waiting for another player...";
	
	// TODO: setup actual server??
	server_socket = connect_to_server("localhost");
	args.server_socket = server_socket;
	args.state = &state;

	state.mode = VERSUS;
	game_state_reset(&state);

	print_message_with_esc(waiting_msg);
	// Enable semi-non-blocking reads of user input
	halfdelay(1);

	// Poll server for whether second player has been found, while also allowing
	// for user to disconnect at any time
	while (recv(server_socket, &player_id, sizeof(EPlayer), MSG_DONTWAIT) == -1 ) {
		ch = getch();
		if (ch == ESC_KEY) {
			cbreak();
			close(server_socket);
			return game_over_status;
		}
	}
	// Second player has now been found.

	// TODO: draw countdown here
	get_countdown(server_socket, player_id);

	gui_init(&gui, &state);
	if (pthread_create(&gui_t, NULL, &run_gui, &gui)) {
		printf("Could not run gui thread");
	}
	// This needs to be sent only once
	int well_max[] = {
		state.well_max_x[player_id],
		state.well_max_y[player_id],
	};

	int status = send(server_socket, well_max, sizeof(int) * 2, MSG_NOSIGNAL);
	if (status == -1) {
		perror("sending well_max");
	}
	// else if (status == 0) {
	// 	// This means server closed its socket prematurely (due to other player d/c)
	// 	game_over_status = PLAYER_DISCONNECT;

	// }

	if (pthread_create(&recv_t, NULL, &client_recv_thread, &args))
		printf("Could not run periodic thread\n");

	while ((ch = getch()) != QUIT_KEY) {
		if (send(server_socket, &ch, sizeof(int), MSG_NOSIGNAL) == -1) {
			game_over_status = PLAYER_DISCONNECT;
			break;
		}
		if (state.game_over_flag) {
			game_over_status = state.game_over_flag;
			break;
		}
	}

	// Reads are now blocking
	cbreak();

	pthread_cancel(recv_t);
	if (pthread_join(recv_t, NULL))
		printf("Could not properly terminate periodic thread\n");
	close(server_socket);

	pthread_cancel(gui_t);
	gui_cleanup(&gui, VERSUS);

	return game_over_status;
}
