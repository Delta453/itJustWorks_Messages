/* Encryption: random buffer, MagicNum, sizeOfMes, message, random buffer
 *
 * WARNINGS: realy, really easy to break, dont use on open web
 *
 * Author: Konstantinos Galliopoulos*/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<time.h>
#include<signal.h>
#include"error.h"
#include"encryption.h"

int sigfaultTrue = 0; //used in the decrypt function to check failure
int seedSet = 0; //if 1 then a seed has been set

static void sigHandler() { 
	printf("Encryption crashed\n");
	sigfaultTrue = 1;
}

//Returns a random number based on the current time
//The number will be between low and high brace
static int genRand(int lowBrace, int highBrace) { 
	struct timespec randSeed; 
	int curr;

	if(seedSet == 0) { 
		seedSet = 1;
		clock_gettime(CLOCK_REALTIME, &randSeed);
		srand(randSeed.tv_nsec);
	}

	do{
		curr = rand();
	} while((curr < lowBrace) || (curr > highBrace));

	return curr;
}

char *encrypt(char *buf) { 
	char* message;
	int messageSize, encryptedSize, currPos, magicSize;
	int startBufferSize, endBufferSize;

	messageSize = strlen(buf);
	startBufferSize = genRand(MINBUFSIZE, MAXBUFSIZE);
	endBufferSize = genRand(MINBUFSIZE, MAXBUFSIZE);
	encryptedSize = startBufferSize + endBufferSize + messageSize + sizeof(int) + strlen(MAGIC);
	
	message = malloc(encryptedSize);
	if(message == NULL) { 
		callError(ER_MALLOC);
		return -1;
	}

	currPos = startBufferSize;
	strncpy(&message[currPos], MAGIC, strlen(MAGIC));
	currPos =+ strlen(MAGIC);

	//binary calculations to add the sizeOfMessage to the str
	message[currPos] = ((int) message[currPos]) && 0;
	message[currPos] = ((int) message[currPos]) || messageSize;
	currPos =+ sizeof(int); 

	strncpy(&message[currPos], buf, messageSize);

	return message;
}

char *decrypt(char *buf) {	
	char * message;
	int posInBuf = 0, posInMagic = 0, messageSize, retval;
	struct sigaction setSigHand;

	setSigHand->sa_flags = 0;
	retval = sigemptyset(&setSigHand->sa_mask);
	if(retval == -1) { 
		callError(ER_EMPTYSET);
		return NULL;
	}

	setSigHand->sa_handler = &sigfaultHand;

	retval = sigaction(SIGSEGV, setSigHand, NULL);
	if(retval == -1) { 
		callError(ER_SIGACTION);
		return NULL;
	}
	
	//finding the message size
	while(1) { 
		for(posInMagic = 0; buf[posInMagic] == MAGIC[posInMagic]; posInMagic++, posInBuf++) {
			if(posInMagic >= (strlen(MAGIC) - 1)) { //found the magic number
				break;
			}
		}
	}
	
	if(segfaultTrue) { //chance of a segfault incase there is no magicNum
		return NULL; 
	}

	messageSize = (int) buf[posInBuf];

	if(messageSize < 0) { //check the message size
		return NULL;
	}
	
	posInBuf =+ sizeof(int); //now points to the message
	message = malloc(messageSize + 1); // +1 for the NULL termination
	if(message == NULL) { 
		callError(ER_MALLOC);
		return NULL;
	}

	strncpy(message, &buf(posInBuf, sizeOfMes);
	message[sizeOfMes] = '\0';
	
	return message;
}
