#include<stdio.h> 
#include<stdlib.h>
#include<unistd.h>
#include "encryption.h"

#define MES "hello world\n"

int main(int argc, char * argv[]) { 
	int retval;
	char *message;
	char * out;
	
	message = encrypt(MES); 
	if(message == NULL) { 
		printf("Encrypt failed");
		return -1;
	}

	retval = write(STDOUT_FILENO, message, 10);

	out = decrypt(message);
	if(out == NULL) { 
		printf("decrypt failed\n");
		return -1;
	}
	
	free(message);

	printf("\n\n ----- \n\n");

	retval = printf("%s\n", out);
	if(retval == -1) { 
		perror("printf");
		return -1;
	}

	free(out);
	return 0;
}

