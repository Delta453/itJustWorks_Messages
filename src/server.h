#ifndef HSERVER
#define HSERVER

#include<pthread.h>

#define BACKLOGMAX 500 //sets the maximum request count on the listening thread
//in microseconds, sets the time that threads sleep incase they are waiting
#define WAITTIME 100000
#define LISTENPORTNUM "50001" //the port number that the listen socket is gonna be connected to  default: "50001"
#define CLIENTPORTNUM "50002" //the port number that the client listen socket is binded to default: "50002"

//Constants used for CLI of the server
#define CLI_BOOTUP "Server is being set up, to close the program type 'q'"
#define CLI_SETUPDONE "Server can now recieve requests from users"
#define CLI_SHUTDOWN "Server has shut down"

struct flaggedPipe { 
	pthread_mutex_t used; //locked when any thread is using the pipes
	int pipefd[2];
};

typedef struct { //the first socket connection to the client is the receive
	int receive; //an fd to the socket that the host receives messages from the client
	int send; //an fd to the socket meant to that messages are sent to the client, set to -1 until its created
} clientSocket_t;

typedef struct node { 
	struct node *next;
	struct node *prev;
	clientSocket_t client;
} node_t;

struct flaggedList {
	pthread_mutex_t removal; //locked when any thread is using the list
	node_t *head;
};

/* Returns the fd to a  socket that is in listening mode, -1 incase of failure*/
extern int getListenSocket();

/* Creates the user-thread
 *
 * Returns: -1 incase of failure to set up the thread*/
extern int setUserThread();

/* Reads STDIN until it read 'q', when it does it changes the var shutDownOrdered to 1 and returns*/
extern void readForQuit();

/* Checks if the listening socket has a socket waiting and if it does it creates the cleint-thread for it
 * Returns: 1 incase the backlog is empty, 0 in other cases*/
extern int acceptClients(int socketToCheck);

/* The function for the clienthread
 * Parameters: The node to the client that the thread is responsible for
 * Returns: -1 on failure, 0 on success*/
extern void client_threadFunction(node_t *clientNode);

/* Sets up the flush thread
 * Returns: -1 on failure, 0 on sucess*/
extern int setFlushThread();

/* Description: sends the message to all the user except the sender
 * Parameters: 
 * 	skipfd: the sender fd 
 * 	message: the message to send
 * 	messageSize: size of the message
 *
 * Returns: -1 on failure, 0 on success*/
extern int publishMessage(int skipFd, char *message, int messageSize);

/*Description: The function for the flush_thread*/
extern void flush_threadFunction();

//Frees the clientList
void freeList(node_t *head);
#endif
