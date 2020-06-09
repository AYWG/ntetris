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
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void *client_recv_thread(void *recv_args)
{
	EPlayer i;
	int j, k;
	ClientThreadArgs* args = (ClientThreadArgs *) recv_args;

	int socket = args->server_socket;
	GameState *state = args->state;
	ServerResponse response;

	while (TRUE)
	{
		if (recv(socket, &response, sizeof(ServerResponse), 0) == -1) {
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

void play_ntetris_remote(GameState *local_game_state) {
	int ch;
	int socket_to_server = connect_to_server("localhost");
	EPlayer player_id;
	pthread_t recv_t;
	ClientThreadArgs args;
	args.server_socket = socket_to_server;
	args.state = local_game_state;

	// TODO: check if this is first client or second client
	// Value received from server will indicate player 1 or 2
	if (recv(socket_to_server, &player_id, sizeof(EPlayer), 0) == -1) {
		perror("waiting on both clients");
	}

	// This needs to be sent only once
	int well_max[] = {
		local_game_state->well_max_x[player_id],
		local_game_state->well_max_y[player_id],
	};
	if (send(socket_to_server, well_max, sizeof(int) * 2, MSG_NOSIGNAL) == -1) {
		perror("sending well_max");
	}

	if (pthread_create(&recv_t, NULL, &client_recv_thread, &args))
		printf("Could not run periodic thread\n");

	// Enable semi-non-blocking reads of user input
	halfdelay(1);

	// TODO: deal with other client quitting

	while ((ch = getch()) != QUIT_KEY) {
		if (ch != ERR) {
			if (send(socket_to_server, &ch, sizeof(int), MSG_NOSIGNAL) == -1) {
    		    break;
    		}
		}
		if (local_game_state->game_over_flag) break;
	}

	// Reads are now blocking
	cbreak();

	pthread_cancel(recv_t);
	if (pthread_join(recv_t, NULL))
		printf("Could not properly terminate periodic thread\n");
	// TODO: Notify server that client has quit.
	close(socket_to_server);
}

int connect_to_server(const char * hostname) {
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

	// inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
	// 		s, sizeof s);

	freeaddrinfo(servinfo); // all done with this structure
	return sockfd;
}