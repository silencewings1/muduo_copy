#pragma once
#include "Callbacks.h"
#include "TimerId.h"
#include "base/NonCopyable.h"
#include "datetime/TimeStamp.h"
#include "thread/Thread.h"
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

    void Loop();
    void Quit();

    TimeStamp PollReturnTime() const { return poll_return_time; }

    void RunInLoop(const Functor& cb);
    void QueueInLoop(const Functor& cb);

    TimerId RunAt(const TimeStamp& time, const TimerCallback& cb);
    TimerId RunAfter(const Duration& delay, const TimerCallback& cb);
    TimerId RunEvery(const Duration& interval, const TimerCallback& cb);

    void WakeUp();
    void UpdateChannel(Channel* channel);

    void AssertInLoopThread();
    bool IsInLoopThread() const;

private:
    void AbortNotInLoopThread();
    void HandleRead(); // waked up
    void DoPendingFunctors();

private:
    bool looping;
    bool quit;
    bool calling_pending_functors;

    const ThreadID thread_id;
    std::unique_ptr<Poller> poller;
    std::unique_ptr<TimerQueue> timer_queue;
    TimeStamp poll_return_time;

    const int wakeup_fd;
    std::unique_ptr<Channel> wakeup_channel;

    mutable std::mutex mutex_;
    std::vector<Functor> pending_functors;
};
