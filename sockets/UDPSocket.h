#pragma once

#include <string>
#include <vector>
#include <cstddef>

#include "Socket.h"

namespace dhdylan::Sockets
{
    class UDPSocket : public Socket
    {
        public:

        // @brief creates an IPv4 socket bound to localIP on localPort.
        /// SO_REUSEADDR is automatically enabled upon construction.
        /// if there is an issue during construction an error will be added to the error stack.
        /// @param localIP ip that this socket will be bound to
        /// @param port port that this socket will be bound to
        UDPSocket(std::string localIP, int localPort) : Socket(localIP, localPort, SocketType::UDP) { }; 

        bool SendTo(
            const void* data,
            const size_t size,
            const std::string& destinationIP,
            uint16_t destinationPort,
            ssize_t& bytesSent);

        bool RecvFrom(
            void* buffer,
            size_t maxSize,
            ssize_t& bytesRead,
            std::string& senderIP,
            uint16_t& senderPort);

        /// @brief Joins the multicast group referred to by this socket's bound IP address.
        /// This method will fail if the IP this socket is bound to 
        /// is not within multicast range.
        /// @param interfaceIP This is the IP address to the local interface that you want to join 
        /// the multicast group on.
        /// Remember, when multicast packets are sent, they are addressed to a multicast address,
        /// NOT an IP address that refers to an actual network interface.
        /// Calling this method just tells the kernel "I want to listen for multicast packets that 
        /// are ADDRESSED to the multicast address/port that this socket is bound to,
        /// I want to listen for it on THIS interface. 
        bool JoinMulticastGroup(const std::string& groupIP, const std::string& interfaceIP);

        bool SetMulticastTTL(int timeToLive);

        inline bool IsMulticast() { return isMulticast_; }

        protected:

        bool isMulticast_;
    };
}