#include "Acceptor.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "SocketsOps.h"

Acceptor::Acceptor(EventLoop* loop, const InetAddress& addr)
    : loop(loop)
    , accept_socket(sockets::createNonblockingOrDie())
    , accept_channel(loop, accept_socket.fd())
    , listening(false)
{
    accept_socket.SetResueAddr(true);
    accept_socket.Bind(addr);
    accept_channel.SetReadCallback([this]() { HandleRead(); });
}

void Acceptor::Listen()
{
    loop->AssertInLoopThread();

    listening = true;
    accept_socket.Listen();
    accept_channel.EnableReading();
}

void Acceptor::HandleRead()
{
    loop->AssertInLoopThread();

    auto [conn_fd, addr] = accept_socket.Accept();
    if (conn_fd >= 0)
    {
        if (conn_cb)
        {
            conn_cb(conn_fd, addr);
        }
        else
        {
            sockets::close(conn_fd);
        }
    }
}