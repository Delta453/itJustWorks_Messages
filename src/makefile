BASIC_FLAGS = -Wall -g 
EXTRA_FLAGS = -fsanitize=address
OBJECTFILES = encryption.o error.o

test: encryption.o testEncrypt.c
	gcc $(BASIC_FLAGS) $(EXTRA_FLAGS) encryption.o testEncryption.o -o test

testEncrypt.o: testEncryptionc.
	gcc $(BASIC_FLAGS) encryption.c -c

encryption.o: encryption.c encryption.h
	gcc $(BASIC_FLAGS) $(EXTRA_FLAGS) encryption.c -c 

error.o: error.c error.c 
	gcc $(BASIC_FLAGS) $(EXTRA_FLAGS) error.c -c
