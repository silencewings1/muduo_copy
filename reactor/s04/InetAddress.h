#pragma once
#include <netinet/in.h>
#include <string>

class InetAddress
{
public:
    using Addr = struct sockaddr_in;

public:
    explicit InetAddress(uint16_t port);
    InetAddress(const std::string& ip, uint16_t port);
    InetAddress(const Addr& addr);

    std::string ToHostPort() const;
    const Addr& GetSockAddrInet() const { return addr; }

private:
    Addr addr;
};