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
	ServerSendThreadArgs *args = (ServerSendThreadArgs *) send_args;

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

		if (send(args->client_sockets[PLAYER_1], &response, sizeof(ServerResponse), MSG_NOSIGNAL) == -1) {
			break;
		}

		if (send(args->client_sockets[PLAYER_2], &response, sizeof(ServerResponse), MSG_NOSIGNAL) == -1) {
			break;
		}
	}
}

void *server_recv_thread(void *recv_args)
{
	ServerRecvThreadArgs *args = (ServerRecvThreadArgs *) recv_args;

	int input, numbytes, num_complete_lines;
	EPlayer player_id = args->player_id;
	EPlayer other_player_id = player_id == PLAYER_1 ? PLAYER_2 : PLAYER_1;
	GameState *state = args->state;

	while (TRUE) {
		numbytes = recv(args->client_socket, &input, sizeof(int), 0);
		if (numbytes == -1) {
			perror("server recv");
		}
		if (numbytes <= 0) {
			break;
		}
		pthread_mutex_lock(&(state->tetrimino[player_id].lock));

		if (input == args->controls[MOVE_LEFT]) 
			move_tetrimino(state, player_id, LEFT);
		else if (input == args->controls[MOVE_RIGHT]) 
			move_tetrimino(state, player_id, RIGHT);
		else if (input == args->controls[MOVE_DOWN]) 
			move_tetrimino(state, player_id, DOWN);
		else if (input == args->controls[DROP]) {
			num_complete_lines = drop_tetrimino(state, player_id);
			add_garbage(state, other_player_id, player_id, num_complete_lines);
		}
		else if (input == args->controls[ROTATE_CW])
			rotate_tetrimino(state, player_id, CW);
		else if (input == args->controls[ROTATE_CW])
			rotate_tetrimino(state, player_id, CCW);
		else if (input == args->controls[HOLD])
			hold_tetrimino(state, player_id);

		pthread_mutex_unlock(&(state->tetrimino[player_id].lock));
		usleep(random() % STALL);
	}
}


void run_game(int client_sockets[NUM_PLAYERS])
{
	int controls[NUM_PLAYERS][NUM_CONTROLS] = {
		{KEY_LEFT, KEY_RIGHT, KEY_DOWN, KEY_UP, P_KEY, O_KEY, ENTER_KEY},
		{A_KEY, D_KEY, S_KEY, W_KEY, G_KEY, F_KEY, SPACE_KEY}
	};
	EPlayer i;
	int c;

	GameState state;
	game_state_init(&state, INVALID_DIFFICULTY, VERSUS);
	
	pthread_t periodic_t[NUM_PLAYERS];
	pthread_t lock_in_t[NUM_PLAYERS];
	pthread_t send_t;
	pthread_t recv_t[NUM_PLAYERS];
	
	ThreadArgs thread_args[NUM_PLAYERS];

	ServerSendThreadArgs server_send_args;
	server_send_args.state = &state;

	ServerRecvThreadArgs server_recv_args[NUM_PLAYERS];

	// Get well_max dimensions before actually starting
	int well_max[4];
	if (recv(client_sockets[PLAYER_1], well_max, sizeof(int) * 2, 0) == -1) {
		perror("recv well_max");
	}

	if (recv(client_sockets[PLAYER_2], well_max + 2, sizeof(int) * 2, 0) == -1) {
		perror("recv well_max");
	}

	for (i = PLAYER_1; i < NUM_PLAYERS; i++) {
		state.well_max_x[i] = well_max[i * 2];
		state.well_max_y[i] = well_max[i * 2 + 1];

		init_tetrimino(&state, i, get_rand_num(0, 6));

		thread_args[i].state = &state;
		thread_args[i].player_id = i;
		server_send_args.client_sockets[i] = client_sockets[i];

		if (pthread_create(&periodic_t[i], NULL, &periodic_thread, &thread_args[i]))
			printf("Could not run periodic thread\n");
		if (pthread_create(&lock_in_t[i], NULL, &lock_in_thread, &thread_args[i]))
			printf("Could not run lock in thread\n");

		server_recv_args[i].client_socket = client_sockets[i];
		for (c = 0; c < NUM_CONTROLS; c++)  
			server_recv_args[i].controls[c] = controls[i][c];
		server_recv_args[i].player_id = i;
		server_recv_args[i].state = &state;

		if (pthread_create(&recv_t[i], NULL, &server_recv_thread, &server_recv_args[i]))
			printf("Could not run server recv thread\n");
	}
	if (pthread_create(&send_t, NULL, &server_send_thread, &server_send_args))
		printf("Could not run server send thread\n");

	for (i = PLAYER_1; i < NUM_PLAYERS; i++)
		if (pthread_join(recv_t[i], NULL))
			printf("Could not properly terminate server recv thread\n");

	// Cleanup
	pthread_cancel(send_t);
	if (pthread_join(send_t, NULL))
		printf("Could not properly terminate send thread\n");
	for (i = PLAYER_1; i < NUM_PLAYERS; i++) {
		pthread_cancel(periodic_t[i]);
		if (pthread_join(periodic_t[i], NULL))
			printf("Could not properly terminate periodic thread\n");
		
		pthread_cancel(lock_in_t[i]);
		if (pthread_join(lock_in_t[i], NULL))
			printf("Could not properly terminate lock in thread\n");
	}
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
	EPlayer player_1 = PLAYER_1;
	EPlayer player_2 = PLAYER_2;
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

	sin_size = sizeof their_addr;
	new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
	if (new_fd == -1) {
		perror("accept");
	}

	// inet_ntop(their_addr.ss_family,
	// 	get_in_addr((struct sockaddr *)&their_addr),
	// 	s, sizeof s);
	// printf("server: got connection from %s\n", s);

    // if (send(new_fd, "Waiting for second player", 25, 0) == -1) {
    //     perror("send");
    // }
    sin_size = sizeof their_addr;
    new_fd2 = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd2 == -1) {
        perror("accept");
    }

    // inet_ntop(their_addr.ss_family,
    //     get_in_addr((struct sockaddr *)&their_addr),
    //     s, sizeof s);
    // printf("server: got connection from %s\n", s);

    if (send(new_fd, &player_1, sizeof(EPlayer), 0) == -1) {
        perror("send");
    }

    if (send(new_fd2, &player_2, sizeof(EPlayer), 0) == -1) {
        perror("send");
    }

	int client_sockets[] = {new_fd, new_fd2};

	run_game(client_sockets);

	close(new_fd);
	close(new_fd2);
    close(sockfd);

    return 0;
}