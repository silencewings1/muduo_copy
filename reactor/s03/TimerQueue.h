#pragma once
#include "Callbacks.h"
#include "Channel.h"
#include "base/NonCopyable.h"
#include "datetime/TimeStamp.h"
#include <memory>
#include <set>
#include <vector>

class EventLoop;
class Timer;
class TimerId;

class TimerQueue : NonCopyable
{
private:
    using Entry = std::pair<TimeStamp, Timer*>;
    using TimerList = std::set<Entry>;

public:
    TimerQueue(EventLoop* loop);
    ~TimerQueue();

    TimerId addTimer(const TimerCallback& cb,
                     TimeStamp when,
                     Duration interval);

private:
    void handleRead();

    std::vector<Entry> getExpired(TimeStamp now);
    void reset(const std::vector<Entry>& expired, TimeStamp now);
    bool insert(Timer* timer);

    void addTimerInLoop(Timer* timer);

private:
    EventLoop* loop_;

    const int timerfd_;
    Channel timerfdChannel_;
    TimerList timers_;
};
