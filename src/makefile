BASIC_FLAGS = -Wall -g 
EXTRA_FLAGS = -fsanitize=address

#object files used for both the executables
OBJECTFILES = encryption.o error.o message.o server.o 
EXECUTABLES = server client

server: $(OBJECTFILES) server.o 
	gcc $(BASIC_FLAGS) $(EXTRA_FLAGS) $(OBJECTFILES) server.o -o server

server.o: server.c message.h error.h 
	gcc $(BASIC_FLAGS) $(EXTRA_FLAGS) server.c -c

message.o: message.c message.h error.h encryption.h
	gcc $(BASIC_FLAGS) $(EXTRA_FLAGS) message.c -c 

encryption.o: encryption.c encryption.h error.h
	gcc $(BASIC_FLAGS) $(EXTRA_FLAGS) encryption.c -c 

error.o: error.c error.h
	gcc $(BASIC_FLAGS) $(EXTRA_FLAGS) error.c -c

clean: 
	rm -f $(OBJECTFILES) $(EXECUTABLES)
