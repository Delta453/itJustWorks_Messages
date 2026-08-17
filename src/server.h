#ifndef HSERVER
#define HSERVER

#define PORTNUM "50001"
#define BACKLOGMAX 500 //sets the maximum request count on the listening thread
//in microseconds, sets the time that threads sleep incase they are waiting
//accept-thread: the time it waits in case the backlog is empty
//flush-thread: the time it waits incase the out buffer is empty or the client list is empty
#define WAITTIME 100000

//Constants used for CLI of the server
#define CLI_BOOTUP "Server is being setted up, to close the program type 'q'"
#define CLI_SETUPDONE "Server can now recieve requests from users"
#define CLI_SHUTDOWN "Server has shut down"


struct flaggedPipe { 
	int used; //set to 1 if another thread is using it
	int pipefd[2];
};

typedef struct { //the first socket connection to the client is the receive
	int receive; //an fd to the socket that the host receives messages from the client
	int send; //an fd to the socket meant to that messages are sent to the client, set to -1 until its created
} clientSocket_t;

typedef struct node { 
	struct node *next;
	clientSocket_t client;
} node_t;

/* Returns the fd to a  socket that is in listening mode, -1 incase of failure*/
extern int getListenSocket();

/* Sets up a thread that scans STDIN and when the user types 'q' then it changes the
 * shutdownOrdered var to 1
 *
 * Returns: -1 incase of failure to set up the thread*/
extern int setInputThread();

/* Reads STDIN until it read 'q', when it does it chagnes the var shutDownOrdered to 1 and returns*/
extern void readForQuit();

/* Checks if the listening socket has a socket waiting and if it does it creates the cleint-thread for it
 * Returns: 1 incase the backlog is empty, 0 in other cases*/
extern int acceptClients(int socketToCheck, node_t *list);

/*This is the function for the flush-thread. Its takes input from the toSend pipe, stores it in 
 * a heap buffer and then sends it out to all user's except the one that sent it.*/
extern void sendToClients(node_t **list);

/* Frees the list of clients. The ptr given, after the function points to invalid memory*/
extern void freeList(node_t *head);

/* Description: The function for the client_thread
 * Parameters: a ptr to the client node that the thread is responsible for*/
extern void client_threadFunction(clientSocket_t *client);

/*Description: The function for the flush_thread
 * Parameter: a ptr to the head of the list of client*/
extern void flush_threadFunction(node_t **list);

//to add: 
//all the thread fucntions, 
#endif
