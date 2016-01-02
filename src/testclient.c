#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int sockfd;

static int prepare_socket(char* host, int port) {
	int st; // status

    printf("Creating socket...\n");
	sockfd = socket(AF_INET, SOCK_STREAM, 0); // IPv4, TCP
	if (sockfd < 0)
		exit(1);
    printf("Created socket %d\n", sockfd);

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(host); // htonl(INADDR_ANY);
    printf("Connecting...\n");
	st = connect(sockfd, (struct sockaddr*) &server_addr, sizeof(struct sockaddr));
	if (st < 0)
        exit(1);
    printf("Connected to %d:%d\n", server_addr.sin_addr.s_addr, server_addr.sin_port);

	return 0;
}

static int close_socket() {
	if (close(sockfd) < 0)
		exit(1);
	return 0;
}

static int send_test() {
    int st;

	char* buf = "TEST";

	st = write(sockfd, buf, sizeof buf);
	if (st < 0)
		exit(1);

	return (st>0) ? 1 : 0 ;
}

int main(int argc, char** argv) {

	char* server = "127.0.0.2";
	int port = 4711;

	prepare_socket(server, port);
	send_test();
	close_socket();

	return 0;
}
