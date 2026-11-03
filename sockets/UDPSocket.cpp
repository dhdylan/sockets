#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#include "UDPSocket.h"

bool dhdylan::Sockets::UDPSocket::SendTo(const void* data, const size_t size, const std::string& destinationIP, uint16_t destinationPort, ssize_t &bytesSent)
{
    // reset bytesSent
    bytesSent = 0;
    
    if (data == nullptr)
    {
        errorMessageStack_.push("SendTo() error: data pointer was null.");
        return false;
    }

    sockaddr_in destinationSockAddr{};

    // convert IP string to bytes
    int status = inet_pton(AF_INET, destinationIP.c_str(), &destinationSockAddr.sin_addr);
    if (status != 1)
    {
        if (status == 0)
        {
            errorMessageStack_.push(
                std::string("Invalid string passed into Socket constructor: ") +
                destinationIP);
        }
        else
        {
            errorMessageStack_.push(
                std::string("inet_pton() error: ") +
                strerror(errno));
        }
        
        return false;
    }

    // we're only doing IPv4 for now
    destinationSockAddr.sin_family = AF_INET;

    // set local port
    destinationSockAddr.sin_port = htons(destinationPort);

    bytesSent = sendto(
        fileDescriptor_,
        data,
        size,
        0,
        reinterpret_cast<sockaddr*>(&destinationSockAddr),
        sizeof(destinationSockAddr));

    if (bytesSent == -1)
    {
        // TODO
        // we should be handling this somehow
        // maybe it is done by the caller or maybe internally, not worrying about it rn though
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            bytesSent = 0;
            return true;
        }

        errorMessageStack_.push(
            std::string("sendto() error: ") +
            strerror(errno));
            return false;
    }

    return true;
}

bool dhdylan::Sockets::UDPSocket::RecvFrom(void* buffer, size_t maxSize, ssize_t &bytesRead, std::string &senderIP, uint16_t &senderPort)
{
    sockaddr_in senderSockAddr{};
    socklen_t senderSockAddrLen = sizeof(senderSockAddr);
    
    bytesRead =  recvfrom(
        fileDescriptor_,
        buffer,
        maxSize,
        0,
        reinterpret_cast<sockaddr*>(&senderSockAddr),
        &senderSockAddrLen);
    
    if (bytesRead == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            bytesRead = 0;
        }
        else
        {
            errorMessageStack_.push(
                std::string("recvfrom() error: ") +
                strerror(errno));
                return false;
        }
    }

    if (bytesRead > 0)
    {
        char address[INET_ADDRSTRLEN] {};
        if (inet_ntop(AF_INET, &senderSockAddr.sin_addr, address, sizeof(address)) == nullptr)
        {
            errorMessageStack_.push(
                std::string("inet_ntop() error: ") +
                strerror(errno));
                return false;
        }

        senderIP.assign(address);

        senderPort = ntohs(senderSockAddr.sin_port);
    }    

    return true;
}

bool dhdylan::Sockets::UDPSocket::JoinMulticastGroup(const std::string& groupIP, const std::string& interfaceIP)
{
    if (!open_)
    {
        errorMessageStack_.push("JoinMulticastGroup error: Socket has not been opened.");
        return false;
    }

    ip_mreq mreq{};

    // set the multicast group address on the multicast structure
    // i'm checking this before the next block just because this is the more fundamental error
    if (inet_pton(AF_INET, groupIP.c_str(), &mreq.imr_multiaddr) != 1 ||
        !IN_MULTICAST(ntohl(mreq.imr_multiaddr.s_addr)))
    {
        errorMessageStack_.push(
            "JoinMulticastGroup error: group IP is not a valid multicast address: " + localIP_);
        return false;
    }

    // check if the socket's local ip isn't 0.0.0.0
    // if it isn't, it needs to be the same as the group addresss specified here or
    // no traffic would actually get received
    if (localIP_ != "0.0.0.0" && localIP_ != groupIP)
    {
        errorMessageStack_.push("JoinMultiastGroup() error: Socket is bound to a non-any address "
            "that did not match the groupIP.");
        return false;
    }

    // set the local interface to listen on
    if (inet_pton(AF_INET, interfaceIP.c_str(), &mreq.imr_interface) != 1)
    {
        errorMessageStack_.push(
            "JoinMulticastGroup error: invalid interface IP: " + interfaceIP);
        return false;
    }

    if (setsockopt(fileDescriptor_, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
    {
        errorMessageStack_.push(
            std::string("setsockopt(IP_ADD_MEMBERSHIP) error: ") + strerror(errno));
        return false;
    }

    isMulticast_ = true;
    return true;
}

bool dhdylan::Sockets::UDPSocket::SetMulticastTTL(int timeToLive)
{
    if (!open_)
    {
        errorMessageStack_.push("SetMulticastTTL error: Socket has not been opened.");
        return false;
    }

    if (timeToLive < 0 || timeToLive > 255)
    {
        errorMessageStack_.push("SetMulticastTTL error: TTL must be 0-255.");
        return false;
    }

    // unsigned char is the portable type for this option
    unsigned char ttl = static_cast<unsigned char>(timeToLive);
    if (setsockopt(fileDescriptor_, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0)
    {
        errorMessageStack_.push(
            std::string("setsockopt(IP_MULTICAST_TTL) error: ") + strerror(errno));
        return false;
    }

    return true;
}