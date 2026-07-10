BASIC_FLAGS = -Wall -g 
EXTRA_FLAGS = -fsanitize=address
OBJECTFILES = encryption.o error.o #object files for the final executable
TESTOBJECTFILES = testEncryption.o

test: encryption.o testEncryption.o error.o
	gcc $(BASIC_FLAGS) $(EXTRA_FLAGS) error.o encryption.o testEncryption.o -o test

testEncryption.o: testEncryption.c encryption.h
	gcc $(BASIC_FLAGS) $(EXTRA_FLAGS) testEncryption.c -c

encryption.o: encryption.c encryption.h error.h
	gcc $(BASIC_FLAGS) $(EXTRA_FLAGS) encryption.c -c 

error.o: error.c error.h
	gcc $(BASIC_FLAGS) $(EXTRA_FLAGS) error.c -c

clean: 
	rm -f $(OBJECTFILES)
	rm -f $(TESTOBJECTFILES)
