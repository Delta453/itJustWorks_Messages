BASIC_FLAGS = -Wall -g 
EXTRA_FLAGS = 

#object files used for both the executables
OBJECTFILES = encryption.o error.o message.o
#object files not common across all executables
EXECOBJECTFILES = server.o client.o
EXECUTABLES = server client

reset:
	make clean
	make client
	make server

client: $(OBJECTFILES) client.o
	gcc $(BASIC_FLAGS) $(EXTRA_FLAGS) $(OBJECTFILES) client.o -o client

server: $(OBJECTFILES) server.o 
	gcc $(BASIC_FLAGS) $(EXTRA_FLAGS) $(OBJECTFILES) server.o -o server

client.o: message.h encryption.h error.h client.c 
	gcc $(BASIC_FLAGS) $(EXTRA_FLAGS) client.c -c

server.o: server.c encryption.h error.h message.h error.h 
	gcc $(BASIC_FLAGS) $(EXTRA_FLAGS) server.c -c

message.o: message.c message.h error.h encryption.h
	gcc $(BASIC_FLAGS) $(EXTRA_FLAGS) message.c -c 

encryption.o: encryption.c encryption.h error.h
	gcc $(BASIC_FLAGS) $(EXTRA_FLAGS) encryption.c -c 

error.o: error.c error.h
	gcc $(BASIC_FLAGS) $(EXTRA_FLAGS) error.c -c

clean: 
	rm -f $(OBJECTFILES) $(EXECOBJECTFILES) $(EXECUTABLES)
