#pragma once

class Timer;

class TimerId
{
public:
    explicit TimerId(Timer* timer)
        : value_(timer)
    {
    }

private:
    Timer* value_;
};
