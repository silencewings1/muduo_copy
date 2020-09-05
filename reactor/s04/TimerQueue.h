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
    using Entry = std::pair<TimeStamp, std::shared_ptr<Timer>>;
    using TimerList = std::set<Entry>;

public:
    TimerQueue(EventLoop* loop);
    ~TimerQueue();

    TimerId AddTimer(const TimerCallback& cb,
                     TimeStamp when,
                     Duration interval);

private:
    void HandleRead();

    std::vector<Entry> GetExpired(TimeStamp now);
    void Reset(const std::vector<Entry>& expired, TimeStamp now);
    bool Insert(std::shared_ptr<Timer> timer);

    void AddTimerInLoop(std::shared_ptr<Timer> timer);

private:
    EventLoop* loop;

    const int timer_fd;
    Channel timer_channel;
    TimerList timers;
};
