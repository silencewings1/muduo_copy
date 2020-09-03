#pragma once
#include "base/NonCopyable.h"
#include <condition_variable>
#include <mutex>
#include <thread>

class EventLoop;

class EventLoopThread : NonCopyable
{
public:
    EventLoopThread();
    ~EventLoopThread();

    EventLoop* StartLoop();

private:
    void Task();

private:
    EventLoop* loop;

    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
};
