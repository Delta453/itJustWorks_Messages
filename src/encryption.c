/* Encryption format: random buffer, MagicNum, sizeOfMes, message, random buffer
 * Warning: This is a very basic encryption and should not be used for any reason other than testing*/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<time.h>
#include<signal.h>
#include"error.h"
#include"encryption.h"

int seedSet = 0; //if 1 then a seed has been set

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

	curr = rand();
	curr = (curr % highBrace) + lowBrace;
	if(curr > MAXBUFSIZE) { 
		curr = MAXBUFSIZE;
	}

	return curr;
}

char *encrypt(char *buf, int bufferSize, int *encryptedSize) { 
	char* message;
	int currPos;
	int startBufferSize, endBufferSize;

	startBufferSize = genRand(MINBUFSIZE, MAXBUFSIZE);
	endBufferSize = genRand(MINBUFSIZE, MAXBUFSIZE);
	*encryptedSize = startBufferSize + endBufferSize + bufferSize + sizeof(int) + strlen(MAGIC);
	
	message = malloc(*encryptedSize);
	if(message == NULL) { 
		callError(ER_MALLOC);
		return NULL;
	}

	currPos = startBufferSize;
	// memcpy to avoid error, source: stackoverflow, user: dbush
	memcpy(&message[currPos], MAGIC, strlen(MAGIC));
	currPos += strlen(MAGIC);

	//adds the size of message to the buffer
	memcpy(&message[currPos], &bufferSize, sizeof(int));
	currPos += sizeof(int); 

	memcpy(&message[currPos], buf, bufferSize);

	return message;
}

char *decrypt(char *buf, int encryptedSize) {	
	char * message;
	int messageSize, posInBuf = 0;
	int magicSize = strlen(MAGIC);
	
	for(int curr = 0; posInBuf < encryptedSize; posInBuf++) { 
		if(curr >= magicSize) { //check if at end of MAGIC
			break;
		}

		if( buf[posInBuf] == MAGIC[curr]) { 
			curr++;
		}
		else {
			curr = 0;
		}
	}

	if((posInBuf == encryptedSize) || ((encryptedSize - posInBuf) < sizeof(int))) { 
		return NULL;
	}
	
	memcpy(&messageSize, &buf[posInBuf], sizeof(int));

	if(messageSize < 0) { //check the message size
		return NULL;
	}
	
	posInBuf += sizeof(int); //now points to the message
	message = malloc(messageSize + 1); // +1 for the NULL termination
	if(message == NULL) { 
		callError(ER_MALLOC);
		return NULL;
	}

	memcpy(message, &buf[posInBuf], messageSize);
	message[messageSize] = '\0';
	
	return message;
}
