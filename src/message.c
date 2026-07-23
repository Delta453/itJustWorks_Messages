// Writing the time in the packet is implemented in a wrong way
#include"encryption.h"
#include"message.h"
#include"error.h" 
#include<stdlib.h>
#include<string.h>
#include<time.h>

char *createMessage(char *user, packetType_t type, char *containedMessage) { 
	char *message, *packet;
	int messageSize = 0;

	if(type == PAC_MESSAGE) { 
		messageSize = strlen(containedMessage);
	}
	
	packet = pack(user, type, containedMessage, messageSize);
	if(packet == NULL) { 
		return NULL;
	}

	message = encrypt(packet);
	free(packet);
	return message;
}

char *breakMessage(char *messageToBreak, packetType_t *type, char **containedMessage, unsigned int *messageSize, 
		struct tm **time) { 
	char *packet;
	char *username;

	packet = decrypt(messageToBreak);
	if(packet == NULL) { 
		return NULL;
	}

	username = upack(packet, type, containedMessage, messageSize, time);
	free(packet);
	return username;
}

// Adds the time information to a packet
// Parameters: position is from which point to add the time info
static void addTime(char *packet, struct tm *time, int position) { 
	int intSize = sizeof(int);

	memcpy(&packet[position], &time->tm_year, intSize);
	memcpy(&packet[position], &time->tm_mon, intSize);
	memcpy(&packet[position], &time->tm_mday, intSize);
	memcpy(&packet[position], &time->tm_hour, intSize);
	memcpy(&packet[position], &time->tm_min, intSize);
	memcpy(&packet[position], &time->tm_sec, intSize);
}

// Extracts the time out of a packet
static void readTime(char *packet, struct tm *time) { 
	time->tm_year = (int) packet[0];
	time->tm_mon = (int) packet[4];
	time->tm_mday = (int) packet[8];
	time->tm_hour = (int) packet[12];
	time->tm_min = (int) packet[16];
	time->tm_sec = (int) packet[20];
}

char *pack(char *user, packetType_t type, char *message, unsigned int memSize) {
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
	addTime(retPacket, &timeCurrent, usernameSize);

	//insert the packet type
	reallocRet = realloc(retPacket, currentPackSize + sizeof(packetType_t));
	if(reallocRet == NULL) { 
		callError(ER_REALLOC);
		free(retPacket);
		return NULL;
	}
	retPacket = reallocRet;
	currentPackSize += sizeof(packetType_t);
	retPacket[currentPackSize - sizeof(packetType_t)] = type;

	if(type != PAC_MESSAGE) {
		return retPacket;
	}

	//inserting the message into the packet
	reallocRet = realloc(retPacket, currentPackSize + sizeof(int) + memSize);
	if(reallocRet == NULL) { 
		callError(ER_REALLOC);
		free(retPacket);
		return NULL;
	}

	retPacket = reallocRet;
	retPacket[currentPackSize] = memSize;
	strncpy(&retPacket[currentPackSize + sizeof(int)], message, memSize);
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
	*time = malloc(sizeof(struct tm));
	if(time == NULL) { 
		callError(ER_MALLOC);
		free(username);
		return NULL;
	}
	readTime(&packet[current], *time);
	current += 6*sizeof(int);

	//getting the package type
	*type = (packetType_t) packet[current];

	if(*type != PAC_MESSAGE) { 
		return username;
	}

	current += sizeof(packetType_t);

	//getting the message
	*messageSize = (unsigned int) packet[current];
	current += sizeof(unsigned int);
	*message = malloc(*messageSize + 1);
	if(message == NULL) {
		callError(ER_MALLOC);
		free(username);
		return NULL;
	}

	strncpy(*message, &packet[current], *messageSize);
	message[current + *messageSize] = '\0';
	return username;
}
