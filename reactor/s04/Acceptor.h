#pragma once
#include "Channel.h"
#include "Socket.h"
#include "base/NonCopyable.h"
#include <functional>

class EventLoop;
class InetAddress;

class Acceptor : NonCopyable
{
public:
    using NewConnCallback = std::function<void(int, const InetAddress&)>;

public:
    Acceptor(EventLoop* loop, const InetAddress& addr);

    void SetNewConnCallback(const NewConnCallback& cb) { conn_cb = cb; }
    bool Listening() const { return listening; }
    void Listen();

private:
    void HandleRead();

private:
    EventLoop* loop;
    Socket accept_socket;
    Channel accept_channel;
    NewConnCallback conn_cb;
    bool listening;
};