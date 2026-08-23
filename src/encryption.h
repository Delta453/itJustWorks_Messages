#ifndef ENCRYPTH
#define ENCRYPTH

#define MAGIC "end"
#define MINBUFSIZE 1
#define MAXBUFSIZE 2

/* Encrpyts a message.
 * Parameters: the message to decrypt, must be \0 terminated
 * Returns: Buf: the ptr to the encrypted message which must be freed after, NULL incase of failure*/
extern char *encrypt(char *buf, int bufferSize, int *encryptedSize);

/* Decrypts a message and returns it. The message return is \0 terminated
 * Parameters; an encrypted message, must be NULL terminated to avoid crash
 * Returns: the ptr to the unecrypted message which must be freed after, NULL Incase of failure*/
extern char *decrypt(char *buf, int encryptedSize);

#endif
