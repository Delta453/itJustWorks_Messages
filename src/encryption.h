#ifndef ENCRYPTH
#define ENCRYPTH

#define MAGIC "heaserrwertdafa"
#define MINBUFSIZE 1
#define MAXBUFSIZE 5

/* Encrpyts a message.
 * Parameters: the message to decrypt, must be \0 terminated
 * Returns: the ptr to the encrypted message which must be freed after*/
extern char *encrypt(char *buf);

/* Decrypts a message and returns it. The message return is \0 terminated
 * Parameters; an encrypted message, must be NULL terminated to avoid crash
 * Returns: the ptr to the unecrypted message which must be freed after*/
extern char *decrypt(char *buf);

#endif
