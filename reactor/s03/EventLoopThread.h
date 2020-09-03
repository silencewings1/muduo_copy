#pragma once
#include "base/NonCopyable.h"
#include "thread/Thread.h"
#include <condition_variable>
#include <mutex>

class EventLoop;

class EventLoopThread : NonCopyable
{
public:
    EventLoopThread();
    ~EventLoopThread();

    EventLoop* startLoop();

private:
    void Task();

private:
    EventLoop* loop_;
    bool exiting_;

    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
};
