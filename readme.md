Author: Konstantinos galliopoulos(konstantinos.galli@gmail.com)  

Description: A chat app using the IVp4 protocol. When connected you can send messages that will be sent to all other  
connected clients. This program is written entirely in C resulting in high speed connections.  

Features:  
	*POSIX compliance both for the server and the client  
	*multi threaded:  
	  -client uses: 2 threads  
		-server uses: 3 + (number of clients) threads  
	*custom data package  
	*custom encryption -it is too weak for production code-  
  
**FOLDER EXPLANATION:**  
src: has the source and the make file for the final executables  
tools: has a collection of test's both to ensure parts of the program worked while also  
	providing an environment for things such as multi threading and sockets to be tested.  
	Note that they are a bit rough around the edges  
	
**HOW TO USE:**  
Server: pull the src folder and type "make server". Then run the executable produced  
Client: pull the src folder, open the "client.c" file and edit the HOSTNAME constant,  
	it should be the DNS or the IPv4 of the OS that the server is set up in.  
	Also unblock port 50002 (bash: sudo ufw allow 50002)  
	-if you still have issues unblocking port 50001 sometimes helped"  
	**Do note:** you can only have **one client per OS** otherwise a "bind failed: port already in use" error will be  
	displayed  
How to set up a the DNS:  
	That is entirely up to the user, for purpose on a single client you can set the server hostname as the  
	host name and if you run the client in the same OS it will work just fine. Otherwise I recommend going into  
	you router setting and setting up a custom DNS and then tell your client OS's to resolve through your router for t 	given DNS  
  
Possible improvements:  
	-minimize the size of the packets  
	-GUI  
  
**ARCHITECTURE**  
Description: Explain the choice behind the project  
Client side:  
 	uses two threads each with its own socket. The socket used by the read thread connects to the server whilst the socket of the  
 	write thread is obtained through a connection from the server to client. I figured it was faster and more efficient to change the  
	port rather than have to find to which client the socket belonged to if it connected like the first socket. It does cost more  
 	resource on the client side but that was a sacrifice since the thread-per-client model already was heavy  
Server side:  
  	Client list:  
    	A list was chosen since I wouldn't need to search it to find anything so it was the best choice. I know that because I still havent  
		had a data base class so I am definitely qualified, dont think about it.  
	Thread-per-client:  
 	   	The focus of the project was on the socket programming and the multi threading so it made sense for me to avoid select and poll  
		for the sake of simplicity.  
	Flush thread:  
    	I wanted to force my self to have to deal with a lot of threads trying to write into the same pipe to see race conditions. It was 
		really fun but definitely not the best for efficiency 

Personal experience: This app was developed in my first summer as an Electronics and Computer Engineer undergraduate (2026). It had the goal of 
furthering my understanding in socket programming as well as in multi threading. It was fun diving into the packet
design which did make me understand how communication works. Overall I am really proud of the project and the 
weeks poured into it. 
