#include<stdio.h> 
#include"error.h"

// Calls perror for the given error type
void callError(call_error_t error) { 
	switch(error) { 
		case ER_READ: { 
			perror("Read failed");
			break;
		}
		case ER_WRITE: { 
			perror("Write failed");
			break;
		}
		case ER_PRINTF: { 
			perror("Printf failed");
			break;
		}
		case ER_SCANF: { 
			perror("Scanf failed");
			break;
		}
		case ER_MALLOC: { 
			perror("Malloc failed");
			break;
		}
		case ER_REALLOC: { 
			perror("Realloc failed");
			break;
		}
		case ER_PIPE: { 
			perror("Pipe failed");
			break;
		}
		case ER_EMPTYSET: { 
			perror("sigemptyset failed");
			break;
		}
		case ER_SIGACTION: {
			perror("sigaction failed");
			break;
		}
		case ER_GETTIME: {
			perror("Gettime failed");
			break;
		}
		case ER_SOCKET: {
			perror("Socket failed");
			break;
		}
		case ER_BIND: { 
			perror("Bind failed");
			break;
		}
		case ER_CONNECT: { 
			perror("Connect failed");
			break;
		}
		case ER_ACCEPT: { 
			perror("Accept failed");
			break;
		}
		case ER_PCREATE: {
			perror("pthread_create failed");
			break;
		}
		case ER_PJOIN: { 
			perror("pthread_join faild");
			break;
		}
		case ER_PATTRINIT: { 
			perror("pthread_attr_init failed");
			break;
		}
		case ER_PATTRDE: {
			perror("pthread_attr_destroy failed");
			break;
		}
		case ER_PDETACH: { 
			perror("pthread_detach failed");
			break;
	  }
		default: { 
			//empty for warning
		}
	}
}

