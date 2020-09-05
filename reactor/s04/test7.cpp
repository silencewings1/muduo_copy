#include "Acceptor.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "SocketsOps.h"
#include <iostream>
#include <unistd.h>

// usgae: telnet localhost 9983

int main()
{
    std::cout << "main pid=" << ::getpid() << std::endl;

    EventLoop loop;

    Acceptor acceptor1(&loop, InetAddress(9981));
    acceptor1.SetNewConnCallback([](int sock_fd, const InetAddress& addr) {
        std::cout << "new conn: accept from: " << addr.ToHostPort() << std::endl;

        ::write(sock_fd, "How are you?\n", 13);
        sockets::close(sock_fd);
    });
    acceptor1.Listen();

    Acceptor acceptor2(&loop, InetAddress(9982));
    acceptor2.SetNewConnCallback([](int sock_fd, const InetAddress& addr) {
        std::cout << "new conn: accept from: " << addr.ToHostPort() << std::endl;

        ::write(sock_fd, "l'm 9982\n", 10);
        sockets::close(sock_fd);
    });
    acceptor2.Listen();

    Acceptor acceptor3(&loop, InetAddress(9983));
    acceptor3.SetNewConnCallback([](int sock_fd, const InetAddress& addr) {
        std::cout << "new conn: accept from: " << addr.ToHostPort() << std::endl;

        const auto now = TimeStamp::Now().ToString();
        ::write(sock_fd, now.c_str(), now.size());
        sockets::close(sock_fd);
    });
    acceptor3.Listen();

    loop.Loop();

    return 0;
}