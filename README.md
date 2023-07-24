Could you please implement a simple C++ TCP server that can handle up to 6 TCP clients.

 - For every connected client, a new thread should be spawned to handle the relationship
 - Every second, the thread will generate and send a new unique 32 bit sequence number ID
 - Each generated IDs should be different and unique for the whole day (including throught restarts) and across all clients
 - The ID will be sent (ASCII coded) to the client, the ID will terminated by a new line character. 
 - The ID generation is performance sensitive and cannot be pre-computed.
 - For each received new line character, the server will broadcast the number of clients currently connected, to all connected clients.
 - Ctrl-C will send "Thank you\n" to all the connected clients and immediately exit the server cleanly.
 
 - You should only use basic system libraries (no zmq, no boost...etc...); the standard library is allowed.
 - Please limit the usage of global variables.

No TCP Client is expected. A standard linux TCP Client will be used for the evaluation of your server.
The evaluation is based on simplicity, correctness, performance and code readability.

We would need the source code and the instruction to compile and run it on a linux server.
# ------- Project for Celoxica : TCP Server -------

Pre-requisites :
Cmake, gcc, telnet installed

Compilation :
1 - In a terminal Navigate to the project directory
2 - mkdir build; cd build
3 - cmake .. (-DENABLE_TIMESTAMP=ON to enable time computation display of GenerateId method)
4 - make 

Execution :
In the build repository :
1 - ./Celo 
2 - on differents terminal : telnet <ipV4 address> 12345
3 - kill the telnet terminals to disconnect clients or Ctrl + C on server





GCC 11.3.0
