#pragma once
#include "base/NonCopyable.h"
#include <functional>

class EventLoop;

class Channel : NonCopyable
{
public:
    using EventCallback = std::function<void()>;

public:
    Channel(EventLoop* loop, int fd);

    void HandleEvent();

    void SetReadCallback(const EventCallback& cb) { read_cb = cb; }
    void SetWriteCallback(const EventCallback& cb) { write_cb = cb; }
    void SetErrorCallback(const EventCallback& cb) { error_cb = cb; }

    int fd() const { return fd_; }
    int events() const { return events_; }
    void set_revents(int revt) { revents_ = revt; }

    bool IsNoneEvent() const;
    void EnableReading();

    // for Poller
    int Index() { return index; }
    void SetIndex(int idx) { index = idx; }
    EventLoop* OwnerLoop() { return loop; }

private:
    void Update();

private:
    EventLoop* loop;

    const int fd_;
    int events_;
    int revents_;
    int index; // used by Poller.

    EventCallback read_cb;
    EventCallback write_cb;
    EventCallback error_cb;
};
