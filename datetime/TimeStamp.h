#pragma once
#include <chrono>
#include <string>

using Duration = typename std::chrono::system_clock::duration;
using TimePoint = typename std::chrono::system_clock::time_point;

class TimeStamp
{
public:
    explicit TimeStamp(int64_t ms_epoch);
    explicit TimeStamp(const TimePoint& time_point);

    int64_t MicroSecondsSinceEpoch() const;
    Duration DurationSinceNow() const;

    std::string ToString() const;
    void Swap(TimeStamp& other);
    bool Valid() const;

    static TimeStamp Now();
    static TimeStamp Invalid();

public:
    friend TimeStamp AddTime(TimeStamp origin, const Duration& duration_);

    bool operator<(const TimeStamp& other) const;
    bool operator>(const TimeStamp& other) const;
    bool operator<=(const TimeStamp& other) const;
    bool operator>=(const TimeStamp& other) const;
    bool operator==(const TimeStamp& other) const;

private:
    TimePoint tp;
};
