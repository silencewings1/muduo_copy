#include "Socket.h"
#include "InetAddress.h"
#include "SocketsOps.h"
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <strings.h> // bzero

Socket::Socket(int sock_fd)
    : sock_fd(sock_fd)
{
}

Socket::~Socket()
{
    sockets::close(sock_fd);
}

void Socket::Bind(const InetAddress& addr)
{
    sockets::bindOrDie(sock_fd, addr.GetSockAddrInet());
}

void Socket::Listen()
{
    sockets::listenOrDie(sock_fd);
}

std::tuple<int, InetAddress> Socket::Accept()
{
    struct sockaddr_in saddr;
    bzero(&saddr, sizeof(saddr));
    int conn_fd = sockets::accept(sock_fd, &saddr);
    
    return std::make_tuple(conn_fd, InetAddress(saddr));
}

void Socket::SetResueAddr(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR,
                 &optval, sizeof(optval));
}