#include<stdio.h> 
#include"error.h"

// Calls perror for the given error type
void callError(call_error_t error) { 
	switch(error) { 
		case ER_MALLOC: { 
			perror("Malloc failed");
		}
		case ER_EMPTYSET: { 
			perror("sigemptyset failed");
		}
		case ER_SIGACTION: {
			perror("sigaction failed");
		}
		default: { 
			//empty for warning
		}
	}
}
