This code implements a TCP client-server model. The server maintains an in-memory dictionary and listens for commands from the client to manipulate or retrieve data from this dictionary. The client allows a user to issue these commands and see the results.

Available commands:

- `$ add <key> <value>`: add (key, value) pair, if there is no existing pair with same key value
- `$ getvalue <key>`: return value for the matching key in the (key, value) pair, if any
- `$ getall`: return all (key, value) pairs
- `$ remove <key>`: remove matching (key, value) pair, if any
- `$ quit`: terminate client

# Instructions on how to run the program:

1. Open TWO terminals
2. Run make command in on of the terminals
3. In the terminal for sever, run:

   `$ ./server <port-number>`

   Here, the **port-number** argument has to be an integer between 30000 and 40000

4. In the terminal for client, run:

   ` $ ./client <hostname> <port-number>`

   Here, the **hostname** should be the name of the machine you are running your SERVER on and the **port-number** should be the same as in step 3

5. Now you can do all the operations required in the spec: add, getvalue, getall, remove, quit

## On the client side (client.c):

- I create a single socket **sockfd** that allows us to connect() to the server and send() the commands read from stdin.

- I also make sure that the commands provided by the user are the allowed ones. If so, I send the entire command to the server.

## On the server side (server.c):

- I set up a local socket **sockfd** and bind to the provided port number. This socket will be used to listen() to and accept() incoming connections.

- When a connection has been accept()ed, I initialize a new socket **new_fd**. **new_fd** contains information about the machine whose request has been accept()ed.

- With this info, we can now recv() messages from that machine, parse the received command, maintain the dictionary of values and send() back to that socket if necessary in case of 'getvalue' and 'getall'.

* I also added checks for the existence of elements in the dictionary. If there are no elements and the user commands 'getall', we get a message "No elements in the dictionary". In case of 'getvalue' it would be "No such element in the dictionary" if there are either no elements at all or no element with a provided key.
