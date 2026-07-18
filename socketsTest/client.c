// Returns: -1 on fail and 1 on get address fail
#include<net/if.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h> 

#define DOMAINNAME "pi"
#define PORTNUM 8080
#define MESSAGE "hello world"

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
	int retval, clientfd; 
	struct sockaddr_in6 serverAddr;
	struct addrinfo ivp6Hint, *serverAddrInfo;

	clientfd = socket(AF_INET6, SOCK_STREAM, 0);
	if(clientfd == -1) { 
		perror("socket failed"); 
		return -1;
	}

	//connect to the server
	serverAddr.sin6_family = AF_INET;
	serverAddr.sin6_port = PORTNUM;
	serverAddr.sin6_flowinfo = 0;
	ivp6Hint.ai_family = AF_INET6;
	ivp6Hint.ai_socktype = SOCK_STREAM;
	
	if(getaddrinfo(DOMAINNAME, NULL, &ivp6Hint, &serverAddr) != 0) { 
		printf("Get address failed\n"); 
		return 1;
	}

	if(connect(clientfd, serverAddr, sizeof(struct sockaddr_in6)) == -1) { 
		perror("Connect failed\n");
		return -1;
	}

	//connection should have been established
	retval = sendMessage(clientfd, MESSAGE);
	if(retval == -1) { 
		return -1;
	}

	printf("Message sent: %s\n", MESSAGE); 
	return 0;
}
