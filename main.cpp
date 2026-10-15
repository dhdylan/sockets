/*
** showip.c
**
** show IP addresses for a host given on the command line
*/

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <string>
#include <cerrno>

int RunServer()
{
    struct addrinfo hints{}, *myAddressInfo;
    int status = -1;
    
    hints.ai_family = AF_INET; // i wanna use IPv4
    hints.ai_socktype = SOCK_STREAM; // dont entirely understand why we need to specify sock stream to getaddrinfo, it seems like something you should pass into socket

    // use getaddrinfo to get local IP
    if ((status = getaddrinfo("127.0.0.1", "3490", &hints, &myAddressInfo)) != 0)
    {
        std::cerr << "getaddrinfo failed: " << gai_strerror(status);
        return 1;
    }

    //=================================================
    // now we need to create a socket to listen on
    //=================================================

    int socketFd = socket(myAddressInfo->ai_family, myAddressInfo->ai_socktype, myAddressInfo->ai_protocol);
    if (socketFd == -1)
    {
        std::cerr << "socket failed: " << strerror(errno) << std::endl;
        return 1;
    }

    // enable reuse of this socket address so we dont end up getting EADDRINUSE
    // when we try to launch the server immediately after closing it.
    const int ENABLE = 1;
    if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &ENABLE, sizeof(int)) == -1)
    {
        std::cerr << "setsockopt failed: " << strerror(errno) << std::endl;
        return 1;
    }

    //===============================================================
    // bind the socket to the address and port we specified before
    //===============================================================

    if (bind(socketFd, myAddressInfo->ai_addr, myAddressInfo->ai_addrlen) == -1)
    {
        std::cerr << "bind failed: " << strerror(errno) << std::endl;
        return 1;
    }

    // we're done with the addrinfo struct
    freeaddrinfo(myAddressInfo);

    //==========================
    // start listening
    //==========================

    // remember, listen() is not blocking, this just tells the OS to start listening on this socket
    // the OS handles the TCP handshaking and everything and puts stuff in the queue
    // accept is the call that can be blocking.
    if (listen(socketFd, 10) == -1)
    {
        std::cerr << "listen failed: " << strerror(errno) << std::endl;
        return 1;
    }

    //==========================
    // accept the connection
    //==========================
    
    // remember, we use sockaddr_storage for when a method is going to return a sockaddr somehow
    // but we dont know what flavor of sockaddr its going to be
    // sockaddr in this API is kind of like a C inheritance problem
    // sockaddr is like a "base class", and there are more specific types of sockaddr like sockaddr_in
    // or sockaddr_in6 (IPv4 vs IPv6), amongst others
    // sockaddr_storage is just a struct you can use that promises to be large enough to fit any of the sockaddr types in it
    sockaddr_storage theirAddress{};
    socklen_t lengthOfTheirAddress = sizeof(theirAddress);
    
    // accept will block until the underlying queue has something to accept
    int communicatorSocketFd = accept(
        socketFd,
        (sockaddr*)&theirAddress,
        &lengthOfTheirAddress);

    if(communicatorSocketFd == -1)
    {
        std::cerr << "accept failed: " << strerror(errno) << std::endl;
        return 1;
    }

    // log who was accepted

    char theirAddressStringBuffer[INET6_ADDRSTRLEN];
    sockaddr_in* theirAddressCasted = reinterpret_cast<sockaddr_in*>(&theirAddress);
    inet_ntop(AF_INET, &theirAddressCasted->sin_addr, theirAddressStringBuffer, INET6_ADDRSTRLEN);
    std::cout << "Accepted connection from: " << theirAddressStringBuffer << std::endl;

    //==========================
    // say hi
    //==========================

    // wait a sec before sending the message ...
    sleep(2);

    std::cout << "Sending greeting message to client ..." << std::endl;

    std::string message("Hello from the server!");
    ssize_t sent = send(communicatorSocketFd, message.c_str(), message.size(), 0);
    if(sent == -1)
    {
        std::cerr << "send failed: " << strerror(errno) << std::endl;
        return 1;
    }

    if (static_cast<size_t>(sent) != message.size())
    {
        std::cout << "send did not send all of the bytes for some reason????" << std::endl;
    }

    //==========================
    // recv acknowledge message
    //==========================

    char buffer[256]{};

    int bytesRead = recv(communicatorSocketFd, buffer, sizeof(buffer), 0);
    if (bytesRead == -1)
    {
        std::cerr << "recv failed: " << strerror(errno) << std::endl;
        return 1;
    }
    else if (bytesRead == 0)
    {
        std::cout << "client disconnected." << std::endl;
    }
    else
    {
        // print the message back out
        std::cout << "Message from client: " << std::string(buffer, bytesRead) << std::endl;
    }

    // clean up (???)

    close(socketFd);
    close(communicatorSocketFd);

    return 0;
}

int RunClient()
{
    //=================================================
    // get a sockaddr to use with getaddrinfo
    //=================================================
    
    struct addrinfo hints{}, *serverAddressInfo;
    int status = -1;
    
    hints.ai_family = AF_INET; // i wanna use IPv4
    hints.ai_socktype = SOCK_STREAM; // dont entirely understand why we need to specify sock stream to getaddrinfo, it seems like something you should pass into socket

    // use getaddrinfo to fill out the addrinfo struct
    if ((status = getaddrinfo("127.0.0.1", "3490", &hints, &serverAddressInfo)) != 0)
    {
        std::cerr << "getaddrinfo failed: " << gai_strerror(status);
        return 1;
    }

    //=======================================================
    // create a socket from the address info we filled out
    //=======================================================

    int socketFd = socket(serverAddressInfo->ai_family, serverAddressInfo->ai_socktype, serverAddressInfo->ai_protocol);
    if (socketFd == -1)
    {
        std::cerr << "socket failed: " << strerror(errno) << std::endl;
        return 1;
    }

    // enable reuse of this socket address so we dont end up getting EADDRINUSE
    // when we try to open the same socket after closing it.
    const int ENABLE = 1;
    if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &ENABLE, sizeof(int)) == -1)
    {
        std::cerr << "setsockopt failed: " << strerror(errno) << std::endl;
        return 1;
    }

    //=================================================
    // connect to sockaddr with the created socket
    //=================================================

    if (connect(socketFd, serverAddressInfo->ai_addr, serverAddressInfo->ai_addrlen) == -1)
    {
        std::cerr << "connect failed: " << strerror(errno) << std::endl;
        return 1;
    }

    //=================================================
    // wait for greeting message from server
    //=================================================
    
    char buffer[256]{};
    ssize_t bytesRead{};
    if ((bytesRead = recv(socketFd, buffer, sizeof(buffer), 0)) == -1)
    {
        std::cerr << "recv failed: " << strerror(errno) << std::endl;
        return 1;
    }else if (bytesRead == 0)
    {
        std::cout << "server disconnected." << std::endl;
    }
    else
    {
        // print the message back out
        std::cout << "Message from server: " << std::string(buffer, bytesRead) << std::endl;
    }

    //=================================================
    // send back acknowledgement from here
    //=================================================

    // wait for a sec first...
    sleep(2);

    std::string message("Hi, server! I've received your message.");
    ssize_t bytesSent{};
    if ((bytesSent = send(socketFd, message.data(), message.size(), 0)) == -1)
    {
        std::cerr << "send failed: " << strerror(errno) << std::endl;
        return 1;
    }

    //=================================================
    // close
    //=================================================

    close(socketFd);
    freeaddrinfo(serverAddressInfo);

    return 0;
}

int main(int argc, char *argv[])
{
    // first determine if we are the listener or the connector
    if (argc != 2)
    {
        std::cout <<"usage: sockets <connect|listen>" << std::endl;
        return 0;
    }

    //=================
    // CLIENT LOGIC
    //=================
    if (std::string(argv[1]) == "connect")
    {
        return RunClient();
    }
    //=================
    // SERVER LOGIC
    //=================
    else if (std::string(argv[1]) == "listen")
    {
        return RunServer();
    }
    //=================
    // bad args
    //=================
    else
    {        
        std::cout <<"usage: sockets <connect|listen>" << std::endl;
        return 0;
    }
}