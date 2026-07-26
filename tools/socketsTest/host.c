#include<sys/socket.h>
#include<netinet/in.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<net/if.h>

#define ADDRESS "activeSocket" //address the socket is bind to
#define BACKLOGSIZE 1 //for the listening socket
#define PORTNUM 8080
	
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
	int listenSoc;
	int openSoc, retval, mesSize;
	struct sockaddr_in6 openSocAddr;
	struct sockaddr_in6 listenAddr;
	socklen_t openSocLen;
	char *message;

	listenSoc = socket(AF_INET6, SOCK_STREAM, 0);
	if(listenSoc == -1) { 
		perror("Socket failed\n");
		return -1;
	}

	listenAddr.sin6_family = AF_INET6; //type of protocol
	listenAddr.sin6_port = PORTNUM; // the port number of client must match
	listenAddr.sin6_flowinfo = 0; // info about what is prioritized and type of data trasnfered
	//htons is there to convert from little indian to big indian -if needed-
	listenAddr.sin6_scope_id = htons(if_nametoindex("wlan")); // tells which card to use for local networks
	openSocLen = sizeof(struct sockaddr_in6);

	retval = bind(listenSoc, (struct sockaddr*) &listenAddr, openSocLen);
	if(retval == -1) { 
		perror("Bind failed\n");
		return -1;
	}

	retval = listen(listenSoc, BACKLOGSIZE);
	if(retval == -1) { 
		perror("Listen failed\n");
		return -1;
	}

	openSoc = accept(listenSoc, (struct sockaddr*) &openSocAddr, &openSocLen);
	if(openSoc == -1) { 
		perror("Accept failed\n");
		return -1;
	}

	retval = close(listenSoc);
	if(retval == -1) { 
		perror("Close failed\n");
		return -1;
	}

	//have the correct socket open
	mesSize = strlen("hello world");

	message = malloc(mesSize + 1);
	if(message == NULL) { 
		perror("Malloc failed\n");
		return -1;
	}

	message[mesSize] = '\0';

	rread(openSoc, message, mesSize);
	printf("Read: %s\n", message);

	retval = close(openSoc);
	if(retval == -1) {
		perror("CLose failed\n");
		return -1;
	}

	free(message);
	return 0;
}
