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

void play_ntetris_remote() {
	int ch;

	// TODO: connect to server
	int socket_to_server = connect_to_server("localhost");

	// Enable semi-non-blocking reads of user input
	halfdelay(1);

	while ((ch = getch()) != QUIT_KEY) {
		if (ch != ERR) {
			// TODO: send it to the server.
			if (send(socket_to_server, &ch, sizeof(int), 0) == -1) {
    		    perror("send");
    		}
		}
	}
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

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	// while ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) > 0) {
	//     // perror("recv");
	//     // exit(1);
    //     buf[numbytes] = '\0';
    //     printf("client: received '%s'\n",buf);
	// }

    // printf("Server closed socket\n");

	// close(sockfd);
	return sockfd;
}