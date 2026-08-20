/* Description: the program is meant to be run on the server for a server messanger. It will recieve messages
 * from clients and then transmit them to all other clients currently connected. 
 *
 * Threads: 
 * client-thread: a thread that handles reading from the clients socket and sending it to the toSentBuffer as 
 * well as giving notifications to STDOUT about user signing in and out
 * flush-thread: a thread that takes the contents of the toSentBuffer and then sents them to every user connected
 * user-thread: a thread that reads STDIN and waits for the user to type 'q' and when he does it orders the 
 * 		program to close
 * accept-thread: a thread that accepts new requests made to the listening socket and creates the client-threads
 * for them
 *
 * Execution: First creates a socket in listening mode, then creates the user-thread and then becomes a 
 * accept-thread
 *
 * Author: konstantinos galliopoulos
 * Returns: -1 incase of a sys call failure*/

#define _GNU_SOURCE //required for accept4 function

#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h> 
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include"server.h"
#include"message.h"
#include"error.h"

int shutdownOrdered = 0; //used to tell when the user requested that the server is closed
struct flaggedPipe toSendBuffer; //here clien-threads send messages that are to be broadcasted

int getListenSocket() { 
	int retval;
	int fdToSocket;
	struct addrinfo *address;
	struct addrinfo getAddrHint;

	fdToSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if(fdToSocket < 0) { 
		perror("Listening socket failed: Socket failed");
		return -1;
	}

	//getting the address
	memset(&getAddrHint, 0, sizeof(getAddrHint));
	getAddrHint.ai_family = AF_INET;
	getAddrHint.ai_socktype = SOCK_STREAM;
	getAddrHint.ai_flags = AI_PASSIVE;

	retval = getaddrinfo(NULL, LISTENPORTNUM, &getAddrHint, (struct addrinfo **) &address);
	if(retval) { 
		if(retval != EAI_SYSTEM) { 
			fprintf(stderr, "Listening socket failed: getaddrinfo failed: %s\n", gai_strerror(retval));
		}
		else { 
			perror("Listening socket failed: Getaddrinfo failed");
		}

		return -1;
	}

	if(bind(fdToSocket, (struct sockaddr*) address->ai_addr, sizeof(struct sockaddr_in))) {
		perror("Listening socket failed: Bind failed");
		return -1;
	}
	
	if(listen(fdToSocket, BACKLOGMAX)) { 
		perror("Listening socket failed; Listen failed");
		return -1;
	}

	freeaddrinfo(address);
	return fdToSocket;
}	

int setInputThread() { 
	pthread_t thread;
	pthread_attr_t attributes;

	if(pthread_attr_init(&attributes)) { 
		callError(ER_PATTRINIT);
		return -1;
	}

	if(pthread_create(&thread, &attributes, (void*) &readForQuit, NULL)) { 
		callError(ER_PCREATE);
		return -1;
	}

	if(pthread_detach(thread)) { 
		callError(ER_PDETACH);
		return -1;
	}

	return 0;
}

//user-thread function
//Reads the input of the user and if he types 'q' sets the shutdownOrdered value to 1
void readForQuit() { 
	char input;
	int retval;

	do{
		retval = scanf(" %c", &input);

		if(retval == -1) { 
			callError(ER_SCANF);
			shutdownOrdered = 1; 
			pthread_exit(0);
		}
	} while(input != 'q');

	shutdownOrdered = 1;
	pthread_exit(0);
}

int acceptClients(int socketToCheck, node_t *list) { 
	struct sockaddr_in clientAddr;
	socklen_t clientAddrSize;
	node_t *prevNode, *newNode;
	int newClient;
	pthread_t clientThread;
	pthread_attr_t attr;

	clientAddrSize = sizeof(clientAddr);

	newClient = accept4(socketToCheck, (struct sockaddr *) &clientAddr, &clientAddrSize, SOCK_NONBLOCK); 

	if(newClient < 0) { 
		if((errno == EAGAIN) || (errno == EWOULDBLOCK)) { 
			return -1;
		}
		else { 
			perror("Accept4 failed");
			return -1;
		}
	}

	//find the node the client is going to be saved
	newNode = malloc(sizeof(node_t));

	if(list == NULL) { 
		list = newNode;
	}
	else { 
		for(prevNode = list; prevNode->next != NULL; prevNode = prevNode->next);//get the curr to the prev
		prevNode->next = newNode;
	}

	newNode->next = NULL;
	newNode->client.receive = newClient;
	newNode->client.send = -1;

	if(pthread_attr_init(&attr)) { 
		callError(ER_PATTRINIT);
		exit(-1);
	}

	if(pthread_create(&clientThread, &attr, (void*) &client_threadFunction, &newNode->client)) {
		callError(ER_PCREATE);
		exit(-1);
	}

	if(pthread_detach(clientThread)) {
		callError(ER_PDETACH);
		exit(-1);
	}

	if(pthread_attr_destroy(&attr)) { 
			callError(ER_PATTRDE);
			exit(-1);
	}

	//creates the recieve socket
	newNode->client.send = socket(AF_INET, SOCK_STREAM, 0);
	if(newNode->client.send < 0) { 
		callError(ER_SOCKET);
		exit(-1);
	}

	clientAddr.sin_port = htons(atol(CLIENTPORTNUM));
	if(connect(newNode->client.send, (struct sockaddr*) &clientAddr, sizeof(clientAddr))) { 
		callError(ER_CONNECT);
		exit(-1);
	}

	return 0;
}

//Reads the messanges sent and them writes them in the out pipe
void client_threadFunction(clientSocket_t *client) { 
	int sizeFromClient; //the size of the message coming from the (*client)
	int retval;
	int sizeToPipe;
	char *toPipe; // where the message from the pipe message is stored to be written into the pipe
	char *fromClient; //where the message from the (*client) is stored 
	char *username; //where the username of the user is saved
	packetType_t type; // where the type of messsage is saved, incase it is a login/out

	while(1) { 
		if(shutdownOrdered) { 
			break;
		}

		retval = rread((*client).receive, &sizeFromClient, sizeof(sizeFromClient));
		if(retval < 0) { 
			if((errno != EAGAIN) && (errno != EWOULDBLOCK)) { 
				callError(ER_READ);
				exit(-1);
			}

			//recieve socket is empty
			usleep(WAITTIME);
			continue;
		}

		fromClient = malloc(sizeFromClient);
		if(fromClient == NULL) {
			callError(ER_MALLOC);
			exit(-1);
		}

		retval = rread((*client).receive, fromClient, sizeFromClient);
		if(retval < 0) { 
			callError(ER_READ);
			exit(-1);
		}

		username = breakMessage(fromClient, &type, NULL, NULL, NULL, sizeFromClient);
		
		if(username == NULL) { 
			exit(-1);
		}

		retval = printf("%s: ", username);
		if(retval < 0) { 
			callError(ER_PRINTF);
			exit(-1);
		}

		switch(type) { 
			case PAC_LOGIN: {
				retval = printf("logged in\n");
				break;
			}
			case PAC_ULOG: { 
				retval = printf("logged out\n");
				break;
			}
			default: { 
				//exists to avoid warning
			}
		}

		free(username);

		if(retval < 0) { 
			callError(ER_PRINTF);
			exit(-1);
		}

		toPipe = pipeMessage(fromClient, &sizeToPipe, sizeFromClient, (*client).receive);
		free(fromClient);
		while(1) { //wait to write message into the toSend pipe 
			if(toSendBuffer.used) { 
				usleep(WAITTIME);
				continue;
			}

			toSendBuffer.used = 1;
			retval = wwrite(toSendBuffer.pipefd[1], toPipe, sizeToPipe);
			if(retval < 0) { 
				callError(ER_WRITE);
				exit(-1);
			}

			free(toPipe);
			break;
		}
	}

	close((*client).receive);
	close((*client).send);
	return;
}

//Returns -1 incase of failure, 0 incase of success
int setFlushThread() { 
	pthread_t thread;
	pthread_attr_t attributes;

	if(pthread_attr_init(&attributes)) { 
		callError(ER_PATTRINIT);
		return -1;
	}

	if(pthread_create(&thread, &attributes, (void*) &flush_threadFunction, NULL)) { 
		callError(ER_PCREATE);
		return -1;
	}

	if(pthread_detach(thread)) { 
		callError(ER_PDETACH);
		return -1;
	}

	return 0;
}

// Writes the mesage to all the snd sockets of all the clients in the list expept for the skipFd
// skipFd can be left negative incase, in that case no client will be skipped
int publishMessage(node_t *list, int skipFd, char *message, int messageSize) {
	node_t *curr;
	int retval;

	for(curr = list; curr->next != NULL; curr = curr->next) { 
		if(curr->client.receive == skipFd) {
			continue;
		}

		if(curr->client.send == -1) { //send has not been initialized yet
			continue; 
		}

		retval = wwrite(curr->client.send, message, messageSize);
		if(retval == -1) { 
			return -1;
		}

		retval = wwrite(curr->client.send, message, messageSize);
		if(retval == -1) { 
			return -1;
		}
	}

	return 0;
}

// Constantly checks the toSentBuffer and then sents out the message contained 
// in it to all the clients except for the one that sent it
void flush_threadFunction(node_t **list) { 
	char *buffer;
	int socketFd;
	int retval; 
	int bufferSize;

	while(1) { 
		if(shutdownOrdered) { 
			break;
		}

		if(list == NULL) { 
			usleep(WAITTIME);
			continue;
		}

		buffer = upipeMessage(toSendBuffer.pipefd[0], &socketFd, &bufferSize);
		if(buffer == NULL) { 
			if((errno != EAGAIN) && (errno != EWOULDBLOCK)) { 
				perror("Flush-thread error: ");
				exit(-1);
			}
			
			//pipe is empty
			usleep(WAITTIME);
		}

		retval = publishMessage(*list, socketFd, buffer, bufferSize);
		free(buffer);
		if(retval == -1) { 
			close(toSendBuffer.pipefd[0]);
			close(toSendBuffer.pipefd[1]);
			exit(-1);
		}
	}

	close(toSendBuffer.pipefd[0]);
	close(toSendBuffer.pipefd[1]);
	return;
}

void freeList(node_t *head) { 
	node_t *prev, *curr;

	for(curr = head; curr != NULL; prev = curr, curr = curr->next, free(prev));
}

int main(int argc, char* argv[]) {
	struct flaggedPipe toSendBuffer;
	node_t *clientList = NULL;
	int listeningSocket;

	listeningSocket = getListenSocket();
	if(listeningSocket == -1) {
		return -1;
	}

	if(printf("%s\n", CLI_SETUPDONE) < 0) {
		callError(ER_PRINTF);
	}

	if(setInputThread()) { 
		return -1;
	}

	toSendBuffer.used = 0;
	if(pipe2(toSendBuffer.pipefd, O_NONBLOCK)) { 
		callError(ER_PIPE);
		return -1;
	}

	if(setFlushThread()) { 
		return -1;
	}

	if(printf("%s\n", CLI_BOOTUP) < 0) { 
		callError(ER_PRINTF);
	}

	while(1) { //loop for accepting new clients 
		if(shutdownOrdered) { 
			break;
		}

		if(acceptClients(listeningSocket, clientList)) {
			usleep(WAITTIME); //causes latency on connection but saves resources
		}
	}

	freeList(clientList);
	
	if(printf("%s\n", CLI_SHUTDOWN) < 0) { 
		perror("Shutdown message failed to deliver");
	}

	pthread_exit(0);
}
