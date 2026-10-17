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
    hints.ai_socktype = SOCK_DGRAM; // dont entirely understand why we need to specify sock stream to getaddrinfo, it seems like something you should pass into socket

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

    //==========================
    // UDP recvfrom
    //==========================

    char buffer[256];
    sockaddr_storage senderSockAddr{};
    socklen_t senderSockAddrSize = sizeof(senderSockAddr);
    int bytesRead = -1;
    if ((bytesRead = recvfrom(socketFd, buffer, sizeof(buffer), 0, reinterpret_cast<sockaddr*>(&senderSockAddr), &senderSockAddrSize)) == -1)
    {
        std::cerr << "recvfrom failed: " << strerror(errno) << std::endl;
        return 1;
    }

    //==========================
    // see what was read
    //==========================

    // cast to IPv4 sockaddr
    // in a real application, this hsould first read the family of the sockaddr_storage that was received to then determine which type it should be casted to
    sockaddr_in* senderCastedSockAddr = reinterpret_cast<sockaddr_in*>(&senderSockAddr);
    char senderAddrString[INET_ADDRSTRLEN];
    inet_ntop(senderCastedSockAddr->sin_family, &senderCastedSockAddr->sin_addr, senderAddrString, INET_ADDRSTRLEN);
    std::cout << "Message received from: " << senderAddrString << ":" << senderCastedSockAddr->sin_port << ":: " << buffer << std::endl; 

    //==========================
    // tell them you heard it
    //==========================

    // wait a sec before sending the message ...
    sleep(2);

    std::cout << "Sending greeting message to client ..." << std::endl;

    std::string message("Hello from the server!");
    ssize_t sent = sendto(socketFd, message.data(), message.size(), 0, reinterpret_cast<sockaddr*>(senderCastedSockAddr), sizeof(sockaddr_in));
    if(sent == -1)
    {
        std::cerr << "sendto failed: " << strerror(errno) << std::endl;
        return 1;
    }

    if (static_cast<size_t>(sent) != message.size())
    {
        std::cout << "send did not send all of the bytes for some reason????" << std::endl;
    }

    // clean up (???)

    close(socketFd);

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

    std::string message("Hello server, anyone there?");
    ssize_t bytesSent = -1;
    if ((bytesSent = sendto(socketFd, message.data(), message.size(), 0, serverAddressInfo->ai_addr, serverAddressInfo->ai_addrlen)) == -1)
    {
        std::cerr << "sendto failed: " << strerror(errno) << std::endl;
        return 1;
    }

    //=================================================
    // wait for greeting message from server
    //=================================================
    
    char buffer[256]{};
    ssize_t bytesRead{};
    sockaddr_storage senderSockAddr{};
    if ((bytesRead = recvfrom(socketFd, buffer, sizeof(buffer), 0, )) == -1)
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