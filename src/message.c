#include"encryption.h"
#include"message.h" 
#include"error.h" 
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<string.h>
#include<time.h>

char *createMessage(char *user, packetType_t type, char *containedMessage, int *sizeOfMessage) { 
	char *message, *packet;
	int messageSize = 0;
	int packetSize = strlen(user) + 1+ sizeof(type) + 6*sizeof(int);// +1 for the \0 of the user

	if(type == PAC_MESSAGE) { 
		packetSize += sizeof(int) + strlen(containedMessage);
	}

	if(type == PAC_MESSAGE) { 
		messageSize = strlen(containedMessage);
	}
	
	packet = pack(user, type, containedMessage, messageSize);
	if(packet == NULL) { 
		return NULL;
	}

	message = encrypt(packet, packetSize, sizeOfMessage);
	free(packet);
	return message;
}

char *breakMessage(char *messageToBreak, packetType_t *type, char **containedMessage, unsigned int *messageSize, 
		struct tm **time, int toBreakSize) { 
	char *packet;
	char *username;

	packet = decrypt(messageToBreak, toBreakSize);
	if(packet == NULL) { 
		return NULL;
	}

	username = upack(packet, type, containedMessage, messageSize, time);
	free(packet);
	return username;
}

// Adds the time information to a packet
// Parameters: position is from which point to add the time info
static void addTime(char *packet, struct tm *time) { 
	int intSize = sizeof(int);

	memcpy(&packet[0], &time->tm_year, intSize);
	memcpy(&packet[1*intSize], &time->tm_mon, intSize);
	memcpy(&packet[2*intSize], &time->tm_mday, intSize);
	memcpy(&packet[3*intSize], &time->tm_hour, intSize);
	memcpy(&packet[4*intSize], &time->tm_min, intSize);
	memcpy(&packet[5*intSize], &time->tm_sec, intSize);
}

// Extracts the time out of a packet
static void readTime(char *packet, struct tm *time) { 
	memcpy(&time->tm_year, &packet[0], sizeof(int));
	memcpy(&time->tm_mon, &packet[4], sizeof(int));
	memcpy(&time->tm_mday, &packet[8], sizeof(int));
	memcpy(&time->tm_hour, &packet[12], sizeof(int));
	memcpy(&time->tm_min, &packet[16], sizeof(int));
	memcpy(&time->tm_sec, &packet[20], sizeof(int));
}

char *pack(char *user, packetType_t type, char *message, unsigned int messageSize) {
	char *retPacket; //packet to be returned 
	char *reallocRet;
	int usernameSize = strlen(user);
	int currentPackSize; //size of bytes currently in the packet, doesn include the size of the message part
	struct timespec timeSeconds;
	struct tm timeCurrent;

	//inserting the user name
	retPacket = malloc(usernameSize + 1); //'\0'
	if(retPacket == NULL) { 
		callError(ER_MALLOC);
		return NULL;
	}
	currentPackSize = usernameSize + 1;
	strncpy(retPacket, user, usernameSize);
	retPacket[usernameSize] = '\0';

	//inserting the time
	if(clock_gettime(CLOCK_REALTIME, &timeSeconds)) { 
		callError(ER_GETTIME);
		return NULL;
	}
	gmtime_r(&timeSeconds.tv_sec, &timeCurrent);
	reallocRet = realloc(retPacket, currentPackSize + (sizeof(int)*6)); //6 because Y/M/D/H/M/S
	if(reallocRet == NULL) { 
		callError(ER_REALLOC);
		free(retPacket);
		return NULL;
	}
	currentPackSize += sizeof(int)*6;
	retPacket = reallocRet;
	addTime(&retPacket[usernameSize + 1], &timeCurrent);

	//insert the packet type
	reallocRet = realloc(retPacket, currentPackSize + sizeof(packetType_t));
	if(reallocRet == NULL) { 
		callError(ER_REALLOC);
		free(retPacket);
		return NULL;
	}
	retPacket = reallocRet;
	currentPackSize += sizeof(packetType_t);
	memcpy(&retPacket[currentPackSize - sizeof(packetType_t)], &type, sizeof(packetType_t));

	if(type != PAC_MESSAGE) {
		return retPacket;
	}

	//inserting the message into the packet
	reallocRet = realloc(retPacket, currentPackSize + sizeof(int) + messageSize);
	if(reallocRet == NULL) { 
		callError(ER_REALLOC);
		free(retPacket);
		return NULL;
	}

	retPacket = reallocRet;
	memcpy(&retPacket[currentPackSize], &messageSize, sizeof(int));// insert the message size into the packet
	memcpy(&retPacket[currentPackSize + sizeof(int)], message, messageSize); //insert the message into the packet
	return retPacket;
}

char *upack(char* packet, packetType_t *type, char **message, unsigned int *messageSize, struct tm **time) {
	int current; //holds the position within the package
	int usernameSize;
	char *username;

	//getting the username
	usernameSize = strlen(packet);
	username = malloc(usernameSize + 1);
	if(username == NULL) { 
		callError(ER_MALLOC);
		return NULL;
	}
	strncpy(username, packet, usernameSize);
	username[usernameSize] = '\0';
	current = usernameSize + 1;

	//getting the time
	if(time != NULL) { 
		*time = malloc(sizeof(struct tm));
		if(time == NULL) { 
			callError(ER_MALLOC);
			free(username);
			return NULL;
		}
		readTime(&packet[current], *time);
	}

	current += 6*sizeof(int);

	//getting the package type
	memcpy(type, &packet[current], sizeof(packetType_t));

	if(*type != PAC_MESSAGE) { 
		return username;
	}

	current += sizeof(packetType_t);

	if(message == NULL) { 
		return username;
	}

	//getting the message
	memcpy(messageSize, &packet[current], sizeof(unsigned int));
	current += sizeof(unsigned int);
	*message = malloc(*messageSize + 1);
	if(message == NULL) {
		callError(ER_MALLOC);
		free(username);
		return NULL;
	}

	memcpy(*message, &packet[current], *messageSize);
	(*message)[*messageSize] = '\0';
	return username;
}

char *pipeMessage(char* packet, int *packetSize, int messageSize, int fd) { 
	char *message;

	(*packetSize) = messageSize + sizeof(fd) + sizeof(int);

	message = malloc((*packetSize));
	if(message == NULL) { 
		callError(ER_MALLOC);
		return NULL;
	}

	memcpy(message, packetSize, sizeof(int));
	memcpy(&message[sizeof(int)], &fd, sizeof(int));
	memcpy(&message[2*sizeof(int)], packet, messageSize);
	return message;
}

// Uses the read function and insures that it doenst read less than size
// Returns: -1 incase of failure, 0 incase of success
int rread(int fd, void *dst, int size) { 
	int retval = 0;
	int count = 0;

	do { 
		retval = read(fd, &dst[count], size - count);
		if(retval < 0) { 
			return -1;
		}

		count += retval;
	} while(count < size);

	return 0;
}

int wwrite(int fd, char*src, int size) { 
	int retval;
	int count = 0;

	do { 
		retval = write(fd, &src[count], size - count);
		if(retval < 0) { 
			return -1;
		}

		count += retval;
	} while(count < size);

	return 0;
}

char *upipeMessage(int readFd, int *srcFd, int *packetSize) { 
	char *packet;
	int messageSize; 

	if(rread(readFd, &messageSize, sizeof(int))) { 
		return NULL;
	}

	packet = malloc(messageSize - 2*sizeof(int)); // - 2*sizeof(int)
	if(packet == NULL) {
		callError(ER_MALLOC);
		return NULL;
	}

	if(rread(readFd, srcFd, sizeof(int))) { 
		return NULL;
	}

	if(rread(readFd, packet, messageSize - 2*sizeof(int))) { 
		return NULL;
	}

	*packetSize = messageSize -2*sizeof(int);
	return packet;
}
