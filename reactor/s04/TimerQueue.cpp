#include "TimerQueue.h"
#include "EventLoop.h"
#include "Timer.h"
#include "TimerId.h"
#include "logging/Logging.h"
#include <assert.h>
#include <string.h>
#include <sys/timerfd.h>
#include <unistd.h>

namespace
{

int CreateTimerfd()
{
    int timer_fd = ::timerfd_create(CLOCK_MONOTONIC,
                                   TFD_NONBLOCK | TFD_CLOEXEC);
    if (timer_fd < 0)
    {
        LOG_SYSFATAL << "Failed in timerfd_create";
    }

    return timer_fd;
}

struct timespec TimeSinceNow(TimeStamp when)
{
    using namespace std::chrono;
    using namespace std::chrono_literals;

    auto dur = when.DurationSinceNow();
    if (dur < 100ms)
        dur = 100ms;

    struct timespec ts;
    ts.tv_sec = duration_cast<seconds>(dur).count();
    ts.tv_nsec = duration_cast<nanoseconds>(dur - seconds(ts.tv_sec)).count();

    return ts;
}

void ResetTimerfd(int timer_fd, TimeStamp expiration)
{
    struct itimerspec newValue;
    bzero(&newValue, sizeof(newValue));
    newValue.it_value = TimeSinceNow(expiration);
    int ret = ::timerfd_settime(timer_fd, 0, &newValue, nullptr);
    if (ret)
    {
        LOG_SYSERR << "timerfd_settime()";
    }
}

void ReadTimerfd(int timer_fd, TimeStamp now)
{
    uint64_t howmany;
    ssize_t n = ::read(timer_fd, &howmany, sizeof(howmany));
    LOG_TRACE << "TimerQueue::HandleRead() " << howmany << " at " << now.ToString();

    if (n != sizeof(howmany))
    {
        LOG_ERROR << "TimerQueue::HandleRead() reads " << n << " bytes instead of 8";
    }
}

} // namespace

TimerQueue::TimerQueue(EventLoop* loop)
    : loop(loop)
    , timer_fd(CreateTimerfd())
    , timer_channel(loop, timer_fd)
{
    timer_channel.SetReadCallback([this]() { HandleRead(); });
    timer_channel.EnableReading();
}

TimerQueue::~TimerQueue()
{
    ::close(timer_fd);
}

TimerId TimerQueue::AddTimer(const TimerCallback& cb,
                             TimeStamp when,
                             Duration interval)
{
    auto timer = std::make_shared<Timer>(cb, when, interval);
    loop->RunInLoop([&]() { AddTimerInLoop(timer); });

    return TimerId(timer.get());
}

void TimerQueue::AddTimerInLoop(std::shared_ptr<Timer> timer)
{
    loop->AssertInLoopThread();

    bool earliest_changed = Insert(timer);
    if (earliest_changed)
    {
        ResetTimerfd(timer_fd, timer->Expiration());
    }
}

void TimerQueue::HandleRead()
{
    loop->AssertInLoopThread();

    auto now = TimeStamp::Now();
    ReadTimerfd(timer_fd, now);

    auto expired = GetExpired(now);
    for (auto& exp : expired)
    {
        exp.second->Run();
    }

    Reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::GetExpired(TimeStamp now)
{
    auto it = timers.begin();
    for (; it != timers.end(); ++it)
    {
        if (it->first >= now)
            break;
    }

    std::vector<TimerQueue::Entry> expired;
    std::copy(timers.begin(), it, std::back_inserter(expired));
    timers.erase(timers.begin(), it);

    return expired;
}

void TimerQueue::Reset(const std::vector<Entry>& expired, TimeStamp now)
{
    for (const auto& exp : expired)
    {
        if (exp.second->Repeat())
        {
            exp.second->Restart(now);
            Insert(exp.second);
        }
    }

    if (!timers.empty())
    {
        TimeStamp nextExpire = timers.begin()->second->Expiration();
        if (nextExpire.Valid())
        {
            ResetTimerfd(timer_fd, nextExpire);
        }
    }
}

bool TimerQueue::Insert(std::shared_ptr<Timer> timer)
{
    auto when = timer->Expiration();

    bool earliest_changed = timers.empty() || when < timers.begin()->first;
    auto res = timers.insert(std::make_pair(when, timer));
    assert(res.second);

    return earliest_changed;
}
