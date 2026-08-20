/* A library for creating messages and the packets for them
 *
 * Structure: username, time, package type, message
 * time: year, month, day, hour, minutes, seconds
 * message: if type PAC_MESSAGE message size, message else it is empty
 */
#ifndef MESSAGEH
#define MESSAGEH

#include<time.h>

typedef enum { 
	PAC_MESSAGE = 'm',
	PAC_LOGIN = 'l',
	PAC_ULOG = 'u'
} packetType_t;

/*Creates an encrypted message, should be freed after use
 *
 * Parameters
 * user: a ptr to a NULL ended string which is the name of the user
 * type: the type of the message as defined by the enum
 * message: if the type is PAC_MESSAGE the message must be null terminated and must not contain '\0'
 * , else can be set to NULL
 * meSize: if the type is PAC_MESSAGE the size of the message, else can be left 0
 * sizeOfMessasge: where the size of the encrypted message is going to be saved
 *
 * Returns: A ptr to a non NULL ended message, NULL in case of failure*/
extern char *createMessage(char *user, packetType_t type, char *containedMessage, int *sizeOfMessage);

/*Description: Breaks a packet down to its information, the package given must be a null ended string
 *
 * Parameters
 * package: the null ended packet to break down
 * type: where the type of the package is going to be stored
 * message: if the type is PAC_MESSAGE the message contained, else it can be set to NULL
 * messageSize: if the type is PAC_MESSAGE will contain the size of the message excluding the '\0'
 * time: the time of the OS at the time the message was created, can be set to NULL
 * toBreakSize: the size of the encrypted message
 *
 * Returns: The username, NULL in case of failure*/
extern char *breakMessage(char* messageToBreak, packetType_t *type, char **containedMessage, 
		unsigned int *messageSize, struct tm **time, int toBreakSize);

/*Creates packet without encryption that is NOT null ended. Should be freed after use
 *
 * Parameters
 * user: a ptr to a NULL ended string which is the name of the user
 * type: the type of the message as defined by the enum
 * message: if the type is PAC_MESSAGE the message that will be contained, else can be set NULL
 * messageSize: if the type is PAC_MESSAGE the size of the message, else can be set 0
 *
 * Returns: A ptr to the packet, NULL in case of failure*/
extern char *pack(char *user, packetType_t type, char *message, unsigned int messageSize);

/*Description: Breaks a packet down to its information, the package given must be a null ended string
 *
 * Parameters
 * package: the null ended packet to break down
 * type: where the type of the package is going to be stored
 * message: if the type is PAC_MESSAGE the message contained, else can be set NULL
 * messageSize: if the type is PAC_MESSAGE will contain the size of the message excluding the '\0'
 * time: the time of the OS at the time the packet was created
 *
 * Returns: The username, NULL in case of failure*/
extern char *upack(char* packet, packetType_t *type, char **message, unsigned int *messageSize, struct tm **time);

/*Description: Makes a packet with the following structure: int that contains the size of the message including
 * the int, the message
 *
 * Parameters: 
 * 	packet: the packet that the message carries
 *	packetSize: where the size of the packet is gonna be saved
 *	messageSize: the size of the message to be stored in a packet
 *	fd: the file descriptor that this packet came from
 *
 *	Returns: NULL incase of failure, a ptr to the message incase of success must be freed after use
 * */
extern char *pipeMessage(char *packet, int *packetSize, int messageSize, int fd);

/* Description: Reads from the fd a message made by the pipeMessage function. The unpacks it
 *
 * Parameters: 
 * 	readFd: the fd that this fucntion is going to read from
 * 	srcFd: the fd that points to the socket that the message came from
 * 	packetSize: where the size of the packet contained within the message is stored
 *
 * 	Returns: the packet contained, NULL incase of failure should be freed after use*/
extern char *upipeMessage(int readFd, int *srcFd, int *packetSize);

// Uses the read function and insures that it doesnt read less than size
// Returns: -1 icnase of failure, 0 incase of sucess
extern int rread(int fd, void *dst, int size);

// Uses the write function and guarentess that it will write size bytes
// Returns: -1 incase of failure, 0 incase of success
extern int wwrite(int fd, char *src, int size);
#endif
