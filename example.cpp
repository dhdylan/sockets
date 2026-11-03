#include <Socket.h>
#include <UDPSocket.h>

#include <iostream>
#include <string>
#include <string.h>
#include <unistd.h>

using namespace dhdylan::Sockets;

int main(int argc, char* argv[])
{
    if (argc != 2 || (std::string(argv[1]) != "receive" && std::string(argv[1]) != "send"))
    {
        std::cout << "Usage: " << argv[0] << "<receive|send>" << std::endl;
        return 0;
    }

    if (std::string(argv[1]) == "receive")
    {
        UDPSocket receiverSocket("0.0.0.0", 3490);
        receiverSocket.Open();
        receiverSocket.SetBlocking(false);
        receiverSocket.JoinMulticastGroup("239.0.0.1", "127.0.0.1");

        std::string senderIP;
        uint16_t senderPort;
        std::vector<std::byte> buffer(4096);

        ssize_t bytesRead = 0;
        
        while (true)
        {
            if (!receiverSocket.RecvFrom(buffer.data(), 4096, bytesRead, senderIP, senderPort))
            {
                // an error has occurred
                std::cerr << "UDPSocket::RecvFrom() error: " << receiverSocket.GetErrorMessageStack().top() << std::endl;;
                return -1;
            }

            if (bytesRead == 0) // nothing was there to read
            {
                std::cout << "Nothing received yet, waiting..." << std::endl;
                usleep(100000);
                continue;
            }
            else
            {
                std::cout << "Message received from: " <<
                    senderIP << ":" << senderPort << ": " <<
                    std::string(reinterpret_cast<const char*>(buffer.data()), bytesRead) << std::endl;
                return 0;
            }
        }
    }
    else if (std::string(argv[1]) == "send")
    {
        UDPSocket sendSocket("127.0.0.1", 3500);
        sendSocket.Open();
        sendSocket.SetBlocking(false);
        sendSocket.SetMulticastTTL(32);

        const char* message = "Hello from send side!";

        ssize_t bytesSent = 0;
        bool status = sendSocket.SendTo(message, strlen(message), "239.0.0.1", 3490, bytesSent);

        if (!status)
        {
            std::cerr << "UDPSocket::SendTo() error: " << sendSocket.GetErrorMessageStack().top() << std::endl;
            return 1;
        }
        else
        {
            std::cout << "Succesfully sent out packet with " << bytesSent << " bytes sent." << std::endl;
            return 0;
        }
    }
}