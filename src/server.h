#ifndef HSERVER
#define HSERVER

#define BACKLOGMAX 500 //sets the maximum request count on the listening thread
//in microseconds, sets the time that threads sleep incase they are waiting
//accept-thread: the time it waits in case the backlog is empty
//flush-thread: the time it waits incase the out buffer is empty or the client list is empty
#define WAITTIME 100000
#define LISTENPORTNUM "50001" //the port number that the listen socket is gonna be connected to  default: "50001"
#define CLIENTPORTNUM "50002" //the port number that the client listen socket is binded to default: "50002"

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
	struct node *prev;
	clientSocket_t client;
} node_t;

struct flaggedList {
	int removal; //set to 1 when a thread is deleting a client from
	node_t *head;
};

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
extern int acceptClients(int socketToCheck);

/*This is the function for the flush-thread. Its takes input from the toSend pipe, stores it in 
 * a heap buffer and then sends it out to all user's except the one that sent it.*/
extern void sendToClients();

/* Frees the list of clients. The ptr given, after the function points to invalid memory*/
extern void freeList(node_t *head);

/* Description: The function for the client_thread
 * Parameters: a ptr to the client node that the thread is responsible for*/
extern void client_threadFunction(node_t *clientNode);

/*Description: The function for the flush_thread
 * Parameter: a ptr to the head of the list of client*/
extern void flush_threadFunction();

//to add: 
//all the thread fucntions, 
#endif
