# C++ Socket library design

I'm going to be creating a C++ wrapper around the POSIX sockets.h library. This is not intended to be a multiplatform library yet. My plan currently is to divide this into two domains (TCP and UDP), and those will collectively consist of three classes: TcpListener, TcpStream, and UdpSocket. The purpose of TcpListner is *strictly* just to listen for incoming TCP connections and produce fully-connected TcpStream objects. A TcpStream can be created on it's own (not produced by a TcpListener) in the case of a client connecting to a server (TcpListener). In the case of UDP, there isn't really such thing as a "server" or "client" from the perspective of the socket; any UDP socket can send and receive.

In this API you're going to see that everything returns std::error_code. Since exceptions are a big nono in this line of work for a variety of reasons, error handling needs to be handled this way. The idea is that std::error_code will be extended and we can write out own error code classes. I may make this type stricter in the future, but I'm just writing it as `std::error_code` for now for the sake of polymorhpism. This will basically allow us to utilize all the convenience and cleanliness of exceptions (like having a type and having a message built-in with it) without actually having to have the overhead and visual clutter of a try/catch, or falling back to the C-style approach of just having a bunch of constexpr ints representing error codes. Nobody likes that.

# UDP Socket

## Constructor

Args:

- IP address : **char\***
- Port : **int**

Verifies IP internally, creates the socket, always turns on reuse

## Destructor

Gracefully close sockets

## SendTo : std::error_code

Args:

- data : **vector\<std::byte>**
- destinationIP : **char\***
- destinationPort : **int**

## RecvFrom : std::error_code

Args:

- buffer : **vector\<std::bytes>**
- senderIP : **char\*\***
- senderPort : **int\***
- bytesRead : **size_t\***

## JoinMulticastGroup : std::error_code

This method fails if the IP address you set the socket up with is not in the multicast range.

Args:

- interfaceAddress : **char\***
This is the IP address to the local ethernet interface that you want to join the multicast group on. Remember, multicast packets are *addressed to a **multicast** address*. NOT an IP address that refers to an actual ethernet interface. Calling this function just tells the kernel "I want to listen for multicast packets on the multicast address/port that this socket is bound to, listen for it on *this* interface. 

## SetMulticastTTL : std::error_code

Args:

- timeToLive : **int**

## SetBlocking : std::error_code

Args:

- blocking : **bool**