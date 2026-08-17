#ifndef ERROR
#define ERROR

typedef enum{
	ER_READ,
	ER_WRITE,
	ER_PRINTF,
	ER_SCANF,
	ER_MALLOC,
	ER_REALLOC,
	ER_PIPE,
	ER_EMPTYSET,
	ER_SIGACTION, 
	ER_GETTIME,
	ER_SOCKET,
	ER_BIND,
	ER_CONNECT,
	ER_PCREATE,
	ER_PATTRINIT,
	ER_PATTRDE,
	ER_PDETACH
} call_error_t;

/* calls perror and gives it the valid str for the error
 * 
 * Parameters: The enum for the error occured*/
extern void callError(call_error_t error);
#endif
