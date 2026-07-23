// First arugment is the message and the second argument is the ip of the server
#include<net/if.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h> 

#define SERVERDNS "serverIJW.local"
#define PORTNUM "50001"

int sendMessage(int fd, char *message) { 
	int retval, count = 0, mesSize;

	mesSize = strlen(message);

	do {
		retval = write(fd, message, mesSize - count);
		if(retval == -1) { 
			perror("Write failed\n");
			return -1;
		}

		count += retval;
	} while( count < mesSize);

	return 0;
}

int main(int argc, char *argv[]) {
	int clientfd, retval;
	struct addrinfo *serverAddress;
	struct addrinfo getAddrHint;

	printf("message size: %lu\n", strlen(argv[1]));

	clientfd = socket(AF_INET6, SOCK_STREAM, 0);
	if(clientfd == -1) { 
		perror("Socket failed:");
		return -1;
	}

	//setting up the getAddr hint variable
	getAddrHint.ai_flags =0;
	getAddrHint.ai_family = AF_INET6;
	getAddrHint.ai_socktype = SOCK_STREAM;
	getAddrHint.ai_protocol = 0;
	getAddrHint.ai_addrlen = 0;
	getAddrHint.ai_addr = NULL;
	getAddrHint.ai_canonname = NULL;
	getAddrHint.ai_next = NULL;

	retval = getaddrinfo(argv[2], PORTNUM, &getAddrHint, &serverAddress);
	if(retval != 0) {
		fprintf(stderr, "Get address failed: %s\n", gai_strerror(retval));
		return -1;
	}

	if(connect(clientfd, serverAddress->ai_addr, sizeof(struct sockaddr_in6))) { 
		perror("Connect failed:");
		return -1;
	}

	sendMessage(clientfd, argv[1]);

	printf("Message sent, programming closing\n");
	close(clientfd);
	freeaddrinfo(serverAddress);
	return 0;
}
