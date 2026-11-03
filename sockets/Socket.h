#pragma once

#include <string>
#include <stack>
#include <cstdint>

namespace dhdylan::Sockets
{
    enum class SocketType : int
    {
        TCP,
        UDP
    };

    class Socket
    {
        public:

        ~Socket();

        bool SetBlocking(bool blocking);

        virtual bool Open();

        virtual bool Close();

        bool IsOpen() { return open_; }

        inline bool IsBlocking() { return blocking_; }

        inline std::stack<std::string> GetErrorMessageStack() { return errorMessageStack_; }

        protected:

        /// @brief creates an IPv4 socket bound to localIP on localPort.
        /// SO_REUSEADDR is automatically enabled upon construction.
        /// if there is an issue during construction an error will be added to the error stack.
        /// @param localIP ip that this socket will be bound to
        /// @param localPort port that this socket will be bound to
        Socket(std::string localIP, int localPort, SocketType socketType);

        std::string localIP_;
        uint16_t localPort_ =-1;

        int fileDescriptor_ =-1;
        int protocol_ = -1;
        int socketType_ = -1;
        bool blocking_ = true;
        bool open_ = false;

        std::stack<std::string> errorMessageStack_;

        private:

        Socket() = delete;
        Socket(Socket& s) = delete;
        Socket(Socket&& s) = delete;
    };
}