#include<sys/socket.h>
#include<netinet/in.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<net/if.h>
#include<netdb.h>

#define HOSTDNS "pi"
#define BACKLOGSIZE 1 //for the listening socket
#define PORTNUM "50001"
	
void rread(int fd, char *mes, int size) {
	int retval = 0, readCount = 0;

	do {
		retval = read(fd, mes, size);
		if(retval == -1) { 
			perror("Write failed\n");
			exit(-1);
		}

		readCount += retval;
	} while (readCount < size);
}
	

int main(int argc, char *argv[]) { 
	char message[100];
	int serverfd, clientfd;
	struct addrinfo *addrinfo_server;
	struct addrinfo findBindAddrHint;

	serverfd = socket(AF_INET6, SOCK_STREAM, 0);
	if(serverfd == -1) { 
		perror("Socket failed:");
		return -1;
	}

	findBindAddrHint.ai_flags = AI_PASSIVE;
	findBindAddrHint.ai_family = AF_INET6;
	findBindAddrHint.ai_socktype = SOCK_STREAM;
	findBindAddrHint.ai_protocol = 0;
	findBindAddrHint.ai_addrlen = 0;
	findBindAddrHint.ai_addr = NULL;
	findBindAddrHint.ai_canonname = NULL;
	findBindAddrHint.ai_next = NULL;

	if(getaddrinfo(NULL, PORTNUM, &findBindAddrHint, &addrinfo_server)  != 0) { 
		perror("get address failed:");
		return -1;
	}

	if(bind(serverfd, addrinfo_server->ai_addr, sizeof(struct sockaddr_in6))) { 
		perror("bind failed:");
		return -1;
	}

	if(listen(serverfd, BACKLOGSIZE)) { 
		perror("Listen failed:");
		return -1;
	}

	clientfd = accept(serverfd, NULL, NULL);
	if(clientfd == -1) { 
		perror("Accept failed");
		return -1;
	}

	rread(clientfd, message, atol(argv[1]));

	printf("%s\n", message);

	freeaddrinfo(addrinfo_server);
	close(serverfd);
	close(clientfd);
	return 0;
}
