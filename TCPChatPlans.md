# TCP Chat

* Gonna be a server app and a client app  
* Just start the server and it will tell you what port it’s listening on for that room  
* When you run the client, you just specify IP and port, and it will try to connect  
* The server is going to need to poll for incoming traffic.  
  * Upon startup the server will just open up it’s LISTENER SOCKET. The job of this socket is just to sit there and listen for incoming connections. Underneath, the socket only has the server IP/port bound, and the sender IP/port is basically always un-set, since whenever a connection is made, it is accept()ed and a new socket that has all parts of the socket 4-tuple filled out.  
  * Each time a client is accept()ed, the new socket that gets created will be added to the poll file descriptors list, so the poll call will poll for that too. When the poll finds one of these sockets has data to be read, it will recv() the data from that socket.  
  * The listener socket will just be known to always have index 0\. This will be a constant. Any other index in poll file descriptors list will be considered a client, and if there is data to be received on that socket, then that means a message was sent, ~~or they disconnected (bytes received 0\)~~ that is actually handled by poll since you can add that as an event you want to listen for.  
  * I will need to be able to remove sockets from the socket file descriptors list.  
    * Probably a good idea to make this a vector of file descriptors so that it can be manipulated easily, and every time poll() is called, it will be given the vector::data ptr.  
  * Every time data is read from a client, it will need to be sent out to all the other clients. Dont worry about not sending it to the client that sent it r, just deal with there being duplicated text.  
  * Poll is actually just something that operates on file descriptors, and this means that i can poll for STDIN too. So clients have to poll too. One socket is for the server connection which sends and receives messages, but it will have another file descriptor that listens to stdin, and whenever there is something to read on stdin the poll will notify the client app, the client will send out what was written to stdin, and then go back to polling. The poll i think would almost immediately get a packet back from the server basically echoing the message.  
  * Not gonna worry about multicasting yet. Just have a separate socket for each client and do separate sends for each one whenever we need to broadcast a message.

So from server perspective:

1. Open up a socket and bind it to us, thats our listener socket.  
2. Create a pollfd for the listener socket. All pollfd’s (except this one) will listen for POLLIN and POLLHUP.  
3. Add that pollfd to the pollfd list.  
4. Start the main loop. Poll on the pollfd’s list, with it’s count, and no timeout.  
   1. See how many events occurred  
   2. Loop over each pollfd

      Check revents (the events that occurred)

      1. POLLIN?  
         1. Check the file descriptor  
            1. Listener socket file descriptor?  
               1. Accept the connection from the client, which creates a new socket. Add that socket to the client socket list.  
               2. Create a new pollfd (same event subscriptions) for the new client socket and add it to the pollfd list  
               3. Send the client a connected to server message.  
            2. Client socket fd?  
               1. Recv from that socket fd, store message in a buffer.  
                  We’ll do a max message size of 2048 for now, deal with handling partial recv’s later  
               2. Log to server stdout  
               3. Send that same message to each client socket.  
                  Same as above  
      2. POLLHUP  
         1. It’s a client, listener sockets can’t get POLLHUP  
            1. Close the socket  
            2. Remove the socket from the list of sockets and remove the pollfd from the pollfd list.  
               WAIT A SEC MAKE SURE WE’RE OPERATING ON A COPY OF THE POLLFD LIST LOL  
            3. Broadcast a message on all remaining client sockets that so and so disconnected