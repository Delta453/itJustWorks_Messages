#include<stdio.h>
#include<time.h>
#include"message.h"

int main(int argc, char *argv[]) { 
	char *username = argv[1];
	char *messageToBreak;
	char *toSendMessage = "helloWolrdHowIsLife";
	int messageSize;
	packetType_t type = PAC_MESSAGE;
	struct tm *time;

	messageToBreak = createMessage(username, type, toSendMessage);
	username = breakMessage(messageToBreak, &type, &toSendMessage, &messageSize, &time);

	printf("type, username, message contained, message size, time sent(Y/M/D/H/M/S)\n");
	printf("%c, %s, %s, %d, %d/%d/%d/%d/%d/%d\n", type, username, toSendMessage, messageSize, 
			time->tm_year + 1900, time->tm_mon, time->tm_mday, time->tm_hour, time->tm_min, time->tm_sec);

	return 0;
}
