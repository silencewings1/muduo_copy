#pragma once
#include "base/NonCopyable.h"
#include <tuple>

class InetAddress;

class Socket : NonCopyable
{
public:
    explicit Socket(int sock_fd);
    ~Socket();

    int fd() const { return sock_fd; }

    void Bind(const InetAddress& addr);
    void Listen();
    std::tuple<int, InetAddress> Accept(); //(conn, addr)

    void SetResueAddr(bool on);

private:
    const int sock_fd;
};