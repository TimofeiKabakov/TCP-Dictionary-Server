# Project Description:
This code implements a TCP client-server model. The server maintains an in-memory dictionary and listens for commands from the client to manipulate or retrieve data from this dictionary. The client allows a user to issue these commands and see the results.

Available commands:

- `$ add <key> <value>`: add (key, value) pair, if there is no existing pair with same key value
- `$ getvalue <key>`: return value for the matching key in the (key, value) pair, if any
- `$ getall`: return all (key, value) pairs
- `$ remove <key>`: remove matching (key, value) pair, if any
- `$ quit`: terminate client

# Build & Run Instructions:
1. For the sever, run:

   `$ ./server <port-number>`

2. For the client, run:

   ` $ ./client <hostname> <port-number>`

   Here, the **hostname** should be the name of the machine you are running your SERVER on and the **port-number** should be the same as in step 3

# Implementation Details:
### On the client side (client.c):

- I create a single socket **sockfd** that allows us to connect() to the server and send() the commands read from stdin.

- I also make sure that the commands provided by the user are the allowed ones. If so, I send the command to the server.

### On the server side (server.c):

- I set up a local socket **sockfd** and bind to the provided port number. This socket will be used to listen() to and accept() incoming connections.

- When a connection has been accept()ed, I initialize a new socket **new_fd**, which contains information about the machine whose request has been accept()ed.

- With this info, we can now recv() messages from that machine, parse the received command, maintain the dictionary of values and send() back to that socket if necessary
