/* Description: The client side program for a client-server global chat. Inside it run two threads.
 * 	-write_thread: runs in main, reads from the socket connected to the server and displays it to stdout
 * 	is responsible for closing the sockets as well as freeing any memory.
 * 	-read_thread: the thread that sends messages to the server read from stdin
 *
 * Returns: -1 in case of sys/call failure, 0 incase success
*/

#define _GNU_SOURCE //defined for the use of accept4

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h> 
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<errno.h>
#include"message.h"
#include"error.h"

#define MAXSIZE 4000 //the max size a single message can be (in characters)
#define HOSTNAME "delta" // the host name of the server
#define BINDPORTNUM "50002" //the port number used by the client to bind his listen socket, default: 50002
#define SERVERPORTNUM "50001" //the port number used by the listen socket of the server, default: 50001

static int shutdownOrdered = 0; //used to signal the other thread to close, set to 1 to order the other to close

/* Reads from stdin and packets the message using the message.h library and it writes it to 
 * the fd given. Returns 0 when user types q
 *
 * Parameters:
 * 	writefd: the fd where the message is written
 *
 * Returns: -1 on sys call failure, 0 on success*/
int writeToSocket(int writefd) { 
	int retval;
	int messageSize;
	int messageToSendSize;
	char *messageToSend; //stores the result of the create message function
	char message[MAXSIZE + 1]; //stores the input of the user
	char username[21];

	retval = printf("### System: insert your username (max 20 chars)\n"); //getting the username
	if(retval < 0) { 
		callError(ER_PRINTF);
		shutdownOrdered = 1;
		return -1;
	}

	retval = scanf(" %20s", username);  
	if(retval < 0) { 
		callError(ER_SCANF);
		shutdownOrdered = 1;
		return -1; 
	}

	//sending the log in message
	messageToSend = createMessage(username, PAC_LOGIN, NULL, &messageToSendSize); //sending the login request
	if(messageToSend == NULL) { 
		shutdownOrdered = 1;
		return -1;
	}

	retval = wwrite(writefd, (void*) &messageToSendSize, sizeof(messageToSendSize));
	if(retval == -1) { 
		shutdownOrdered = 1;
		return -1;
	}

	retval = wwrite(writefd, messageToSend, messageToSendSize);
	if(retval == -1) { 
		shutdownOrdered = 1;
		return -1;
	}

	//manual for how to sent messages
	retval = printf("### System: To sent messages type it out and press Ctrl + D at the end of it");
	if(retval < 0) { 
		shutdownOrdered = 1;
		callError(ER_PRINTF);
		return -1;
	}

	
	retval = printf("\n(\\n if you want your message to change line)\n");
	if(retval < 0) { 
		shutdownOrdered = 1;
		callError(ER_PRINTF);
		return -1;
	}

	retval = printf("### System: Write !Quit when you want to quit\n");
	if(retval < 0) { 
		shutdownOrdered = 1;
		callError(ER_PRINTF);
		return -1;
	}

	retval = printf("### System: Messages come in the format of:\n");
	if(retval < 0) { 
		shutdownOrdered = 1;
		callError(ER_PRINTF);
		return -1;
	}

	retval = printf("[year/month/day hour/minutes/seconds] username: message\n");
	if(retval < 0) { 
		shutdownOrdered = 1;
		callError(ER_PRINTF);
		return -1;
	}

	while(1) { 
		retval = read(STDIN_FILENO, message, MAXSIZE);
		if(retval < 0) { 
			shutdownOrdered = 1;
			callError(ER_READ);
			return -1;
		}

		message[retval] = '\0';

		if(!strncmp(message, "!Quit", 5)) { //user asks to quit
			shutdownOrdered = 1;
			
			//sending log out message
			messageToSend = createMessage(username, PAC_ULOG, NULL, &messageToSendSize); //sending the login request
			if(messageToSend == NULL) { 
				shutdownOrdered = 1;
				return -1;
			}

			retval = wwrite(writefd, (void*) &messageToSendSize, sizeof(messageToSendSize));
			if(retval == -1) { 
				shutdownOrdered = 1;
				callError(ER_WRITE);
				return -1;
			}

			retval = wwrite(writefd, messageToSend, messageToSendSize);
			if(retval == -1) { 
				shutdownOrdered = 1;
				callError(ER_WRITE);
				return -1;
			}

			break;
		}

		messageSize = retval;
		retval = wwrite(writefd, (void*) &messageSize, sizeof(messageSize));
		if(retval == -1) { 
			callError(ER_WRITE);
			shutdownOrdered = 1;
			return -1;
		}

		messageToSend = createMessage(username, PAC_MESSAGE, message, &messageToSendSize);
		retval = wwrite(writefd, (void*) &messageToSendSize, sizeof(messageToSendSize));
		if(retval == -1) {
			shutdownOrdered = 1;
			return -1;
		}

		retval = wwrite(writefd, messageToSend, messageToSendSize);
		if(retval == -1) { 
			shutdownOrdered = 1;
			return -1;
		}

		free(messageToSend);
		retval = printf("### System: Message sent\n");
		if(retval < 0) { 
			callError(ER_PRINTF);
			return -1;
		}
	}
		
	return 0;
}

/* Checks the socket given and if it has a message it prints it to the user with the following format
 * [year/month/day hour/minutes/seconds] username: message
 *
 * Parameters: 
 * 	readfd: the file descriptor from which to read for a message -must be NON_BLOCKING
 *
 * Returns: -1 on sys/call failure, 0 on success -may not have printed a message*/
int printMessage(int readfd) { 
	int retval = 0;
	char *containedMessage; // if type PAC_MESSAGE then the message contained
	char *message; //the message read from the socket
	char *username;
	packetType_t type;
	struct tm *time;
	unsigned int messageSize; //the size of the message read
	unsigned int containedMessageSize; // if type PAC_MESSAGE the size of the message contained
	
	if(rread(readfd, &messageSize, sizeof(messageSize))) { 
		perror("rread failed");
		return -1;
	}

	if((errno == EWOULDBLOCK) || (errno == EAGAIN)) { 
		return 0;
	}

	if(messageSize < 0) { 
		wwrite(STDERR_FILENO, "### System: message size negative warning\n", 
				strlen("### System: message size negative warning\n"));
		return 0;
	}

	message = malloc(messageSize);
	if(message == NULL) { 
	 callError(ER_MALLOC);
	 return -1;
	}

	if(rread(readfd, message, messageSize)) { 
		perror("rread failed");
		return -1;
	}

	username = breakMessage(message, &type, &containedMessage, &containedMessageSize, &time, messageSize);
	if(username == NULL) {
		return -1;
	}

	if(type != PAC_MESSAGE) { // used so that a log out/in cant be faked by the user
		retval = printf("#System: ");
		if(retval < 0) { 
			callError(ER_PRINTF);
			free(containedMessage);
			free(time);
			return -1;
		}
	}

	retval = printf("[%d/%d/%d %d/%d/%d] %s: ", time->tm_year + 1900, time->tm_mon, time->tm_mday, 
			time->tm_hour, time->tm_min, time->tm_sec, username);

	free(username);
	free(time);
	free(message);

	if(retval == -1) {
		callError(ER_PRINTF);
		free(containedMessage);
		free(time);
		return -1;
	}

	switch(type) { 
		case PAC_MESSAGE: {
			retval = printf("%s", containedMessage);
			free(time);
			if(retval == -1) {
				free(containedMessage);
				callError(ER_PRINTF);
				return -1;
			}

			//adds a \n at the end of the message if one wasnt written in
			if(containedMessage[containedMessageSize -1] == '\n') { 
				retval = wwrite(STDOUT_FILENO, "\n", 1);
				if(retval < 0) { 
					callError(ER_WRITE);
					free(containedMessage);
					return -1;
				}
			}

			free(containedMessage);
			break;
		}
		case PAC_LOGIN: {
			retval = printf("logged in\n");
			if(retval == -1) { 
				free(time);
				callError(ER_PRINTF);
				return -1;
			}
			
			break;
		}
		case PAC_ULOG: { 
			retval = printf("logged out\n");
			if(retval == -1) { 
				free(time);
				callError(ER_PRINTF);
				return -1;
			}
			
			break;
		}
	}

	free(time);

	return 0;
}

int printConnection() { //prints a notification to STDOUT that connection to server was successful
	int retval = 0;

	retval = printf("### System: Connected with the server successfully\n");
	if(retval < 0) { 
		callError(ER_PRINTF);
		return -1;
	}

	retval = printf("### System: Type 'q' to close the program");
	if(retval < 0) { 
		callError(ER_PRINTF);
		return -1;
	}

	return 0;
}

int main(int argc, char *argv[]) { 
	int retval = 0;
	int threadRetval;
	pthread_t readThread;
	pthread_attr_t readThreadAttr;
	
	//socket variables
	int findReadSocket = 0; // the read socket connects to this socket 
	int readSocket = 0;
	int writeSocket = 0; //fd's of the sockets connected to the server
	socklen_t acceptedAddrLen;
	struct addrinfo *clientAddr; // the address of the client/user to bind it to the findReadSocket
	struct addrinfo clientAddrHint;
	struct addrinfo *serverAddr; // the address of the server needed to connect to it
	struct addrinfo serverAddrHint;
	struct addrinfo acceptedAddr; // used to check the accepted socket is of the right type

	findReadSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(findReadSocket < 0) { 
		callError(ER_SOCKET);
		return -1;
	}
	
	//getting the address of the client
	memset(&clientAddrHint, 0, sizeof(clientAddrHint));
	clientAddrHint.ai_family = AF_INET;
	clientAddrHint.ai_socktype = SOCK_STREAM;
	clientAddrHint.ai_flags = AI_PASSIVE;

	retval = getaddrinfo(NULL, BINDPORTNUM, &clientAddrHint, (struct addrinfo**) &clientAddr);
	if(retval) { 
		if(retval != EAI_SYSTEM) { 
			fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(retval));
		}
		else { 
			perror("Getaddrinfo failed");
		}

		return -1;
	}

 	retval = bind(findReadSocket, (struct sockaddr*) clientAddr->ai_addr, sizeof(struct sockaddr_in));
	if(retval == -1) { 
		callError(ER_BIND);
		freeaddrinfo(clientAddr); 
		return -1;
	}

	retval = listen(findReadSocket, 5);
	if(retval < 0) { 
		callError(ER_LISTEN);
		freeaddrinfo(clientAddr); 
		return -1;
	}

	freeaddrinfo(clientAddr); //listen socket is done

	//getting the server address
	memset(&serverAddrHint, 0, sizeof(clientAddrHint));
	serverAddrHint.ai_family = AF_INET;
	serverAddrHint.ai_socktype = SOCK_STREAM;

 	retval = getaddrinfo(HOSTNAME, SERVERPORTNUM, &serverAddrHint, (struct addrinfo**) &serverAddr);
	if(retval) { 
		if(retval != EAI_SYSTEM) { 
			fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(retval));
			freeaddrinfo(serverAddr);
		}
		else { 
			perror("Getaddrinfo failed");
			freeaddrinfo(serverAddr);
		}

		freeaddrinfo(serverAddr);
		return -1;
	}

	writeSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(writeSocket < 0) { 
		callError(ER_SOCKET);
		freeaddrinfo(serverAddr);
		return -1;
	}

	retval = connect(writeSocket, (struct sockaddr*) serverAddr->ai_addr, sizeof(struct sockaddr_in));
	if(retval) { 
		callError(ER_CONNECT);
		freeaddrinfo(serverAddr);
		return -1;
	}

	freeaddrinfo(serverAddr);

	retval = pthread_attr_init(&readThreadAttr);
	if(retval < 0) { 
		callError(ER_PATTRINIT);
		return -1;
	}

	retval = pthread_create(&readThread, &readThreadAttr, (void*) &writeToSocket, writeSocket);
	if(retval < 0) { 
		callError(ER_PCREATE);
		return -1;
	}

	do { //insure the readSocket is AF_INET type
		acceptedAddrLen = sizeof(struct sockaddr_in);
		readSocket = accept4(findReadSocket, (struct sockaddr*) &acceptedAddr, &acceptedAddrLen, SOCK_NONBLOCK);
		if(readSocket < 0) { 
			callError(ER_ACCEPT);
			close(readSocket);
			close(findReadSocket);
			close(writeSocket);
			shutdownOrdered = 1;
			pthread_join(readThread, NULL);
			return -1;
		}

		if((acceptedAddrLen == sizeof(struct sockaddr_in)) && (acceptedAddr.ai_family == AF_INET)) { 
			break;
		}

		close(readSocket);
	} while(1);

	close(findReadSocket);
	retval = printConnection();
	if(retval < 0) { 
		return -1;
	}

	// loop that prints a message from the backlog and checks for possibility for failure or shutdownOrdered
	while(1) { 
		retval = printMessage(readSocket);
		if(retval < 0) { //printing message fails
			close(readSocket);
			close(writeSocket);
			shutdownOrdered = 1;
			if(pthread_join(readThread, NULL)) {
				callError(ER_PJOIN);
				return -1;
			}
		}

		if(shutdownOrdered) { 
			if(pthread_join(readThread, (void**) &threadRetval)) { 
				callError(ER_PJOIN);
			}

			break;
		}
	}

	close(readSocket);
	close(writeSocket);
	return threadRetval; 
}
