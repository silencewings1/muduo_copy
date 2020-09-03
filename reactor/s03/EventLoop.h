#pragma once
#include "base/NonCopyable.h"
#include "Callbacks.h"
#include "datetime/TimeStamp.h"
#include "thread/Thread.h"
#include "TimerId.h"
#include <memory>
#include <mutex>
#include <vector>

class Channel;
class Poller;
class TimerQueue;

class EventLoop : NonCopyable
{
public:
    using Functor = std::function<void()>;

public:
    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    TimeStamp pollReturnTime() const { return pollReturnTime_; }

    void runInLoop(const Functor& cb);
    void queueInLoop(const Functor& cb);

    TimerId runAt(const TimeStamp& time, const TimerCallback& cb);
    TimerId runAfter(const Duration& delay, const TimerCallback& cb);
    TimerId runEvery(const Duration& interval, const TimerCallback& cb);

    void wakeup();
    void updateChannel(Channel* channel);

    void assertInLoopThread()
    {
        if (!isInLoopThread())
        {
            abortNotInLoopThread();
        }
    }

    bool isInLoopThread() const { return threadId_ == Tid(); }

private:
    void abortNotInLoopThread();
    void handleRead(); // waked up
    void doPendingFunctors();

private:
    using ChannelList = std::vector<Channel*>;

    bool looping_;                /* atomic */
    bool quit_;                   /* atomic */
    bool callingPendingFunctors_; /* atomic */

    const ThreadID threadId_;
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;
    TimeStamp pollReturnTime_;

    const int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;

    mutable std::mutex mutex_;
    std::vector<Functor> pendingFunctors_;

    ChannelList activeChannels_;
};
