#ifndef ERROR
#define ERROR

typedef enum{
	ER_MALLOC,
	ER_EMPTYSET,
	ER_SIGACTION
} call_error_t;

/* calls perror and gives it the valid str for the error
 * 
 * Parameters: The enum for the error occured*/
extern void callError(call_error_t error);
#endif
