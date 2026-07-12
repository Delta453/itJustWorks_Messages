#include<stdio.h>
#include<signal.h>
#include<pthread.h>
#include<unistd.h>

int singaled = 0;

void signalHand() { 
	printf("Sig is up\n");
	singaled++;
}

int secondThreadFun() { 
	printf("Heart beat of second thread\n");
	while(1) {
		printf(".");
		sleep(1);

		if(singaled == 2) { 
			break;
		}
	}

	pthread_exit( (void*) 2);
}


int main(int argc, char *argv[]) { 
	int retval;
	pthread_t secondThreadId;
	pthread_attr_t attrSecond;
	struct sigaction sigActHolder;
	sigset_t blockMask;

	sigActHolder.sa_flags = 0;
	sigemptyset(&sigActHolder.sa_mask);
	sigActHolder.sa_handler = &signalHand;
	sigaction(SIGUSR1, &sigActHolder, NULL);

	pthread_attr_init(&attrSecond);
	pthread_create(&secondThreadId, &attrSecond, (void*) &secondThreadFun, NULL);

	sigfillset(&blockMask); // used for the main thread to not get pinged
	pthread_sigmask(SIG_BLOCK, &blockMask, NULL);

	sleep(2);
	pthread_kill(secondThreadId, SIGUSR1);
	sleep(2);
	kill(getpid(), SIGUSR1);
	sleep(2);
	pthread_join(secondThreadId, &retval);

	printf("thread returned: %d\n", retval);
	return 0;
}
