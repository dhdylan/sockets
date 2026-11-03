#pragma once

#include <string>
#include <vector>
#include <cstddef>
#include <optional>

#include "TCPStream.h"
#include "Socket.h"

namespace dhdylan::Sockets
{
    class TCPListener : Socket
    {
        public:
        
        // @brief creates an IPv4 socket bound to localIP on localPort.
        /// SO_REUSEADDR is automatically enabled upon construction.
        /// if there is an issue during construction an error will be added to the error stack.
        /// @param localIP ip that this socket will be bound to
        /// @param port port that this socket will be bound to
        TCPListener(std::string localIP, int localPort, unsigned int backlogSize) : Socket(localIP, localPort, SocketType::TCP) { };

        // i typically return bool as success but since this method intends to return something that
        // is constructed inside of it, i returned std::optional so i could avoid a heap allocation
        std::optional<TCPStream> Accept();

        protected:

        unsigned int backlogSize_;
    };
}