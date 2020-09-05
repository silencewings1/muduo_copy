#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread()
    : loop(nullptr)
{
}

EventLoopThread::~EventLoopThread()
{
    loop->Quit();
    thread_.join();
}

EventLoop* EventLoopThread::StartLoop()
{
    thread_ = std::thread([this]() { this->Task(); });

    {
        std::unique_lock lock(mutex_);
        cond_.wait(lock, [this]() { return loop != nullptr; });
    }

    return loop;
}

void EventLoopThread::Task()
{
    EventLoop loop_new;

    {
        std::unique_lock lock(mutex_);
        loop = &loop_new;
        cond_.notify_one();
    }

    loop_new.Loop();
}
