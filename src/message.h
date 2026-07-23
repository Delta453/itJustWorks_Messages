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
 * message: if the type is PAC_MESSAGE the message that will be contained, else can be set to NULL
 * meSize: if the type is PAC_MESSAGE the size of the message, else can be left 0
 *
 * Returns: A ptr to a non NULL ended message, NULL in case of failure*/
extern char *createMessage(char *user, packetType_t type, char *containedMessage);

/*Description: Breaks a packet down to its information, the package given must be a null ended string
 *
 * Parameters
 * package: the null ended packet to break down
 * type: where the type of the package is going to be stored
 * message: if the type is PAC_MESSAGE the message contained, else it can be set to NULL
 * messageSize: if the type is PAC_MESSAGE will contain the size of the message excluding the '\0'
 * time: the time of the OS at the time the message was created
 *
 * Returns: The username, NULL in case of failure*/
extern char *breakMessage(char* messageToBreak, packetType_t *type, char **containedMessage, unsigned int *messageSize, struct tm **time);

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

#endif
