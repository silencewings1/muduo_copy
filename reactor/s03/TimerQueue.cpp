#define __STDC_LIMIT_MACROS
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

int createTimerfd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                   TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0)
    {
        LOG_SYSFATAL << "Failed in timerfd_create";
    }
    return timerfd;
}

struct timespec howMuchTimeFromNow(TimeStamp when)
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

void readTimerfd(int timerfd, TimeStamp now)
{
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof(howmany));
    LOG_TRACE << "TimerQueue::HandleRead() " << howmany << " at " << now.ToString();
    if (n != sizeof(howmany))
    {
        LOG_ERROR << "TimerQueue::HandleRead() reads " << n << " bytes instead of 8";
    }
}

void resetTimerfd(int timerfd, TimeStamp expiration)
{
    // wake up loop by timerfd_settime()
    struct itimerspec newValue;
    struct itimerspec oldValue;
    bzero(&newValue, sizeof(newValue));
    bzero(&oldValue, sizeof(oldValue));
    newValue.it_value = howMuchTimeFromNow(expiration);
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if (ret)
    {
        LOG_SYSERR << "timerfd_settime()";
    }
}

} // namespace

TimerQueue::TimerQueue(EventLoop* loop)
    : loop_(loop)
    , timerfd_(createTimerfd())
    , timerfdChannel_(loop, timerfd_)
{
    timerfdChannel_.SetReadCallback([this]() { HandleRead(); });
    timerfdChannel_.EnableReading();
}

TimerQueue::~TimerQueue()
{
    ::close(timerfd_);

    for (auto& timer : timers_)
    {
        delete timer.second;
    }
}

TimerId TimerQueue::addTimer(const TimerCallback& cb,
                             TimeStamp when,
                             Duration interval)
{
    Timer* timer = new Timer(cb, when, interval);
    loop_->RunInLoop([&]() { addTimerInLoop(timer); });

    return TimerId(timer);
}

void TimerQueue::addTimerInLoop(Timer* timer)
{
    loop_->AssertInLoopThread();
    bool earliestChanged = insert(timer);

    if (earliestChanged)
    {
        resetTimerfd(timerfd_, timer->expiration());
    }
}

void TimerQueue::HandleRead()
{
    loop_->AssertInLoopThread();

    TimeStamp now(TimeStamp::Now());
    readTimerfd(timerfd_, now);

    std::vector<Entry> expired = getExpired(now);

    // safe to callback outside critical section
    for (std::vector<Entry>::iterator it = expired.begin();
         it != expired.end(); ++it)
    {
        it->second->run();
    }

    reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(TimeStamp now)
{
    std::vector<Entry> expired;
    Entry sentry = std::make_pair(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    TimerList::iterator it = timers_.lower_bound(sentry);
    assert(it == timers_.end() || now < it->first);
    std::copy(timers_.begin(), it, back_inserter(expired));
    timers_.erase(timers_.begin(), it);

    return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, TimeStamp now)
{
    for (std::vector<Entry>::const_iterator it = expired.begin();
         it != expired.end(); ++it)
    {
        if (it->second->repeat())
        {
            it->second->restart(now);
            insert(it->second);
        }
        else
        {
            // FIXME move to a free list
            delete it->second;
        }
    }

    if (!timers_.empty())
    {
        TimeStamp nextExpire = timers_.begin()->second->expiration();
        if (nextExpire.Valid())
        {
            resetTimerfd(timerfd_, nextExpire);
        }
    }
}

bool TimerQueue::insert(Timer* timer)
{
    bool earliestChanged = false;
    TimeStamp when = timer->expiration();
    TimerList::iterator it = timers_.begin();
    if (it == timers_.end() || when < it->first)
    {
        earliestChanged = true;
    }
    std::pair<TimerList::iterator, bool> result =
        timers_.insert(std::make_pair(when, timer));
    assert(result.second);
    return earliestChanged;
}
