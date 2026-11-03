#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sstream>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "Socket.h"

dhdylan::Sockets::Socket::Socket(std::string localIP, int localPort, SocketType socketType) :
    localIP_(localIP),
    localPort_(localPort)
{
    switch(socketType)
    {
        case SocketType::TCP:
            protocol_ = IPPROTO_TCP;
            socketType_ = SOCK_STREAM;
            break;
        case SocketType::UDP:
            protocol_ = IPPROTO_UDP;
            socketType_ = SOCK_DGRAM;
            break;
        default:
            errorMessageStack_.push(
                (std::stringstream("Invalid value ") << 
                static_cast<int>(socketType) <<
                " into SocketType switch").str());
            return;
    }    
}

dhdylan::Sockets::Socket::~Socket()
{
    Close();
}

bool dhdylan::Sockets::Socket::Open()
{
    //==============================================
    // create socket
    //==============================================

    if ((fileDescriptor_ = socket(AF_INET, socketType_, protocol_)) == -1)
    {
        errorMessageStack_.push(
            std::string("socket() error: ") +
            strerror(errno));
        return false;
    }
    
    //==============================================
    // enable reuse
    //==============================================
    
    const int ENABLE = 1;
    if (setsockopt(
        fileDescriptor_, 
        SOL_SOCKET, SO_REUSEADDR,
        &ENABLE, sizeof(ENABLE)) < 0) 
    {
        errorMessageStack_.push(
            std::string("setsockopt() error: ") +
            strerror(errno));

        Close();
        
        return false;
    }

    //==============================================
    // bind
    //==============================================

    sockaddr_in localSockAddr{};

    // convert IP string to bytes
    int status = inet_pton(AF_INET, localIP_.c_str(), &localSockAddr.sin_addr);
    if (status != 1)
    {
        if (status == 0)
        {
            errorMessageStack_.push(
                std::string("Invalid string passed into Socket constructor: ") +
                localIP_);
        }
        else
        {
            errorMessageStack_.push(
                std::string("inet_pton() error: ") +
                strerror(errno));
        }

        Close();
        
        return false;
        
    }
        
    // we're only doing IPv4 for now
    localSockAddr.sin_family = AF_INET;

    // set local port
    localSockAddr.sin_port = htons(localPort_);

    if (bind(fileDescriptor_, reinterpret_cast<sockaddr*>(&localSockAddr), sizeof(localSockAddr)) == -1)
    {
        errorMessageStack_.push(
            std::string("bind() error: ") +
            strerror(errno));

        Close();
        
        return false;
    }
    
    open_ = true;

    return true;
}

bool dhdylan::Sockets::Socket::SetBlocking(bool blocking)
{
    if (!open_)
    {
        errorMessageStack_.push("SetBlocking error: Socket has not been opened.");
        return false;
    }
    // get current flags
    int flags = fcntl(fileDescriptor_, F_GETFL, 0);
    if (flags < 0)
    {
        errorMessageStack_.push(
            std::string("fcntl() error: ") +
            strerror(errno));
        return false;
    }

    // set the blocking bit
    flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);

    // set flags
    if (fcntl(fileDescriptor_, F_SETFL, flags) < 0)
    {
        errorMessageStack_.push(
            std::string("fcntl() error: ") +
            strerror(errno));
        return false;
    }

    blocking_ = blocking;
    return true;
}

bool dhdylan::Sockets::Socket::Close()
{
    if (fileDescriptor_ >= 0)
    {
        close(fileDescriptor_);
    }

    fileDescriptor_ = -1;
    open_ = false;
    return true;

}
