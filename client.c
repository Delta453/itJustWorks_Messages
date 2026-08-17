/* Description: The client side program for a client-server global chat. Inside it run two threads.
 * 	-write_thread: runs in main, reads from the socket connected to the server and displays it to stdout
 * 	is responsible for closing the sockets as well as freeing any memory.
 * 	-read_thread: the thread that sends messages to the server read from stdin
 *
 * Returns: -1 in case of sys/call failure, 0 incase success
*/

#include<stdin.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>

#define MAXSIZE 4000 //the maximum size the message can be in characters

#define HOSTNAME "serverpi" // the host name of the server
#define PORTNUM "50001" //the port number used, default: 50001

int shutdown = 0; //used to signal the other thread to close, set to 1 to order the other to close

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
	char *message[MAXSIZE]; //stores the input of the user
	char *username[21];
	char *scanfFormat[16]; 

	retval = snprintf(scanfFormat, sizeof(scanfFormat), "%%%ds", MAXSIZE);
	if(retval < 0) { 
		perror("snprintf failed\n");
		shutdown = 1;
		return -1;
	}

	retval = printf("### System: insert your username (max 20 chars)\n"); //getting the username
	if(retval < 0) { 
		callError(ER_PRINTF);
		shutdown = 1;
		return -1;
	}

	retval = scanf(" %20s", username);  
	if(retval < 0) { 
		callError(ER_SCANF);
		shutdown = 1;
		return -1; 
	}

	//sending the log in message
	messageToSend = createMessage(username, PAC_LOGIN, NULL, &messageToSendSize); //sending the login request
	if(messageToSend == NULL) { 
		shutdown = 1;
		return -1;
	}

	retval = wwrite(writefd, messageToSendSize, sizeof(messageToSendSize));
	if(retval == -1) { 
		shutdown = 1;
		return -1;
	}

	retval = wwrite(writefd, messageToSend, messageToSendSize);
	if(retval == -1) { 
		shutdown = 1;
		return -1;
	}

	while(1) { 
		retval = scanf(scanfFormat, message);
		if(retval == -1) { 
			callError(ER_SCANF);
			shutdown = 1;
			return -1;
		}

		if(!strncmp(message, "q", 2)) { //user asks to quit
			shudown = 1;
			
			//sending log out message
			messageToSend = createMessage(username, PAC_LOGIN, NULL, &messageToSendSize); //sending the login request
			if(messageToSend == NULL) { 
				shutdown = 1;
				return -1;
			}

			retval = wwrite(writefd, messageToSendSize, sizeof(messageToSendSize));
			if(retval == -1) { 
				shutdown = 1;
				return -1;
			}

			retval = wwrite(writefd, messageToSend, messageToSendSize);
			if(retval == -1) { 
				shutdown = 1;
				return -1;
			}

			break;
		}

		messageSize = retval;
		retval = wwrite(writefd, messageSize, sizeof(messageSize));
		if(retval == -1) { 
			callError(ER_WRITE);
			shutdown = 1;
			return -1;
		}

		messageToSend = createMessage(username, PAC_MESSAGE, message, &messageToSendSize);
		retval = wwrite(writefd, messageToSendSize, sizeof(messageToSendSize));
		if(retval == -1) {
			shutdown = 1;
			return -1;
		}

		retval = wwrite(writefd, messageToSend, messageToSendSize);
		if(retval == -1) { 
			shutdown = 1;
			return -1;
		}

		free(messageToSend);
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
				strlen("### System: message size negative warning\n");
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

	retval = printf("[%d/%d/%d %d/%d/%d] %s: ");

	free(username);
	free(time);
	free(message);

	if(retval == -1) {
		callError(ER_PRINTF);
		free(messageContained);
		return -1;
	}

	switch(type) { 
		case PAC_MESSAGE: {
			retval = printf("%s\n", containedMessage);
			free(containedMessage);
			if(retval == -1) { 
				return -1;
			}

			break;
		}
		case PAC_LOGIC: {
			retval = printf("logged in\n");
			if(retval == -1) { 
				return -1;
			}
			
			break;
		}
		case PAC_ULOG: { 
			retval = printf("logged out\n");
			if(retval == -1) { 
				return -1;
			}
			
			break;
		}
	}

	return 0;
}

void printConnection() { //prints a notification to STDOUT that connection to server was successful
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
}

int main(int argc, char *argv[]) { 
	int writeSocket = 0; //fd's of the sockets connected to the server
	int retval = 0;
	int threadRetval;
	pthread_t readThread;
	pthread_attr_t readThreadAttr;
	
	//socket variables
	int findReadSocket = 0; // the read socket connects to this socket 
	int readSocket = 0
	int writeSocket = 0; //fd's of the sockets connected to the server
	struct sockaddr_in clientAddr; // the address of the client/user to bind it to the findReadSocket
	struct addrinfo clientAddrHint;
	struct sockaddr_in serverAddr; // the address of the server needed to connect to it
	struct addrinfo serverAddrHint;
	struct addrinfo acceptedAddr; // used to check the accepted socket is of the right type

	findReadSocket = socket(AF_INET, SOCK_STREAM, SOCK_NONBLOCK);
	if(findReadSocket < 0) { 
		callError(ER_SOCKET);
		return -1;
	}
	
	//getting the address of the client
	memset(&clientAddrHint, 0, sizeof(clientAddrHint));
	clientAddrHint.ai_family = AF_INET;
	clientAddrHint.ai_socktype = SOCK_STREAM;
	clientAddrHint.ai_flags = AI_PASSIVE;

	retval = getaddrinfo(NULL, PORTNUM, &clientAddrHint, (struct addrinfo*) &clientAddr);
	if(retval) { 
		perror("Get addr of client failed");
		return -1;
	}

 	retval = bind(findReadSocket, &clientAddr, sizeof(clientAddr));
	if(retval == -1) { 
		callError(ER_BIND);
		return -1;
	}

	freeaddrinfo(&clientAddrHint); //listen socket is done

	//getting the server address
	memset(&serverAddrHint, 0, sizeof(clientAddrHint));
	serverAddrHint.ai_family = AF_INET;
	serverAddrHint.ai_socktype = SOCK_STREAM;

 	retval = getaddrinfo(HOSTNAME, PORTNUM, serverAddrHint, (struct addrinfo*) &serverAddr);
	if(retval) { 
		perror("Get addr of server failed");
		return -1;
	}
	
	writeSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(writeSocket < 0) { 
		callError(ER_SOCKET);
		return -1;
	}

	retval = connect(writeSocket, &serverAddr, sizeof(serverAddr));
	if(retval) { 
		callError(ER_CONNECT);
		return -1;
	}

	retval = pthread_init_attr(&readThreadAttr);
	if(retval < 0) { 
		callError(ER_PATTRINIT);
		return -1;
	}

	retval = pthread_create(&readThread, &readThreadAttr, &writeToSocket, writeSocket);
	if(retval < 0) { 
		callError(ER_PCREATE);
		return -1;
	}

	do { //insure the readSocket is AF_INET type
		readSocket = accept(&findReadSocket, acceptedAddr, NULL);
		if(readSocket < 0) { 
			callError(ER_ACCEPT);
			close(readSocket);
			close(findReadSocket);
			close(writeSocket);
			shutdown = 1;
			pthread_join(readThread, NULL);
			return -1;
		}

		if(acceptedAddr.sa_family == AF_INET) { 
			break;
		}

		close(readSocket);
	} while(1);

	close(findReadSocket);
	printConnection();

	while(1) { // loop that prints a message from the backlog and checks for possibility for failure or shutdown
		retval = printMessage(readSocket);
		if(retval < 0) { //printing message fails
			close(readSocket);
			close(writeSocket);
			shutdown = 1;
			if(pthread_join(readThread, NULL)) {
				callError(ER_PJOIN);
				return -1;
			}
		}

		if(shutdown) { 
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
