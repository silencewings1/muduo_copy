#include "InetAddress.h"
#include "SocketsOps.h"
#include <strings.h>

InetAddress::InetAddress(uint16_t port)
{
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = sockets::hostToNetwork32(INADDR_ANY);
    addr.sin_port = sockets::hostToNetwork16(port);
}

InetAddress::InetAddress(const std::string& ip, uint16_t port)
{
    bzero(&addr, sizeof(addr));
    sockets::fromHostPort(ip.c_str(), port, &addr);
}

InetAddress::InetAddress(const Addr& addr)
    : addr(addr)
{
}

std::string InetAddress::ToHostPort() const
{
    char buf[32];
    sockets::toHostPort(buf, sizeof(buf), addr);
    return buf;
}