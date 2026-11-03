#pragma once

#include <string>
#include <vector>
#include <cstddef>

#include "Socket.h"

namespace dhdylan::Sockets
{
    class TCPStream : Socket
    {
        public:

        // @brief creates an IPv4 socket bound to localIP on localPort.
        /// SO_REUSEADDR is automatically enabled upon construction.
        /// if there is an issue during construction an error will be added to the error stack.
        /// @param localIP ip that this socket will be bound to
        /// @param port port that this socket will be bound to
        TCPStream(std::string localIP, int localPort) : Socket(localIP, localPort, SocketType::TCP) { };

        bool Connect(std::string destinationIP, int destinationPort);
        
        bool Read(std::vector<std::byte> buffer);
        bool Write(std::vector<std::byte> data);
    };
}