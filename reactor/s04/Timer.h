#pragma once
#include "Callbacks.h"
#include "base/NonCopyable.h"
#include "datetime/TimeStamp.h"

class Timer : NonCopyable
{
public:
    Timer(const TimerCallback& timer_cb, TimeStamp when, Duration interval)
        : cb(timer_cb)
        , expiration(when)
        , interval(interval)
        , repeat(interval.count() > 0)
    {
    }

    void Run() const
    {
        cb();
    }

    TimeStamp Expiration() const { return expiration; }
    bool Repeat() const { return repeat; }

    void Restart(TimeStamp now)
    {
        if (repeat)
        {
            expiration = AddTime(now, interval);
        }
        else
        {
            expiration = TimeStamp::Invalid();
        }
    }

private:
    const TimerCallback cb;
    TimeStamp expiration;
    const Duration interval;
    const bool repeat;
};
