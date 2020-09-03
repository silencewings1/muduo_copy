#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread()
    : loop_(nullptr)
    , exiting_(false)
{
}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    loop_->quit();
    thread_.join();
}

EventLoop* EventLoopThread::startLoop()
{
    thread_ = std::thread([this]() { this->Task(); });

    {
        std::unique_lock lock(mutex_);
        cond_.wait(lock, [this]() { return loop_ != nullptr; });
    }

    return loop_;
}

void EventLoopThread::Task()
{
    EventLoop loop;

    {
        std::unique_lock lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.loop();
    //assert(exiting_);
}
