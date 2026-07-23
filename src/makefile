BASIC_FLAGS = -Wall -g 
EXTRA_FLAGS = -fsanitize=address
OBJECTFILES = encryption.o error.o message.o #object files for the final executable

test: $(OBJECTFILES) test.o
	gcc $(OBJECTFILES) $(EXTRA_FLAGS) test.o -o test

test.o: message.h test.c 
	gcc $(BASIC_FLAGS) $(EXTRA_FLAGS) test.c -c 

message.o: message.c message.h error.h encryption.h
	gcc $(BASIC_FLAGS) $(EXTRA_FLAGS) message.c -c 

encryption.o: encryption.c encryption.h error.h
	gcc $(BASIC_FLAGS) $(EXTRA_FLAGS) encryption.c -c 

error.o: error.c error.h
	gcc $(BASIC_FLAGS) $(EXTRA_FLAGS) error.c -c

clean: 
	rm -f $(OBJECTFILES)
