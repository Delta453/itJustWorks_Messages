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

	write(STDOUT_FILENO, message, 100);
	out = decrypt(message);
	printf("%s\n", out);
	return 0;
}

