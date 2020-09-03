#pragma once
#include "Callbacks.h"
#include "base/NonCopyable.h"
#include "datetime/TimeStamp.h"

class Timer : NonCopyable
{
public:
    Timer(const TimerCallback& cb, TimeStamp when, Duration interval)
        : callback_(cb)
        , expiration_(when)
        , interval_(interval)
        , repeat_(interval_.count() > 0)
    {
    }

    void run() const
    {
        callback_();
    }

    TimeStamp expiration() const { return expiration_; }
    bool repeat() const { return repeat_; }

    void restart(TimeStamp now)
    {
        if (repeat_)
        {
            expiration_ = AddTime(now, interval_);
        }
        else
        {
            expiration_ = TimeStamp::Invalid();
        }
    }

private:
    const TimerCallback callback_;
    TimeStamp expiration_;
    const Duration interval_;
    const bool repeat_;
};
