#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"
#include "TimerQueue.h"
#include "logging/Logging.h"
#include <assert.h>
#include <sys/eventfd.h>
#include <unistd.h>

namespace
{

thread_local EventLoop* loop_in_this_thread = nullptr;
constexpr int POLL_TIME_MS = 10000;

int createEventfd()
{
    int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (fd < 0)
    {
        LOG_SYSERR << "Failed in eventfd";
        abort();
    }

    return fd;
}

} // namespace

EventLoop::EventLoop()
    : looping(false)
    , quit(false)
    , calling_pending_functors(false)
    , thread_id(Tid())
    , poller(std::make_unique<Poller>(this))
    , timer_queue(std::make_unique<TimerQueue>(this))
    , poll_return_time(TimeStamp::Invalid())
    , wakeup_fd(createEventfd())
    , wakeup_channel(std::make_unique<Channel>(this, wakeup_fd))
{
    LOG_TRACE << "EventLoop created " << this << " in thread " << Tid_64(thread_id);
    if (loop_in_this_thread)
    {
        LOG_FATAL << "Another EventLoop " << loop_in_this_thread
                  << " exists in this thread " << Tid_64(thread_id);
    }
    else
    {
        loop_in_this_thread = this;
    }

    wakeup_channel->SetReadCallback([this]() { HandleRead(); });
    wakeup_channel->EnableReading();
}

EventLoop::~EventLoop()
{
    assert(!looping);
    ::close(wakeup_fd);
    loop_in_this_thread = nullptr;
}

void EventLoop::Loop()
{
    assert(!looping);
    AssertInLoopThread();

    looping = true;
    quit = false;

    Poller::ChannelList active_channels;
    while (!quit)
    {
        active_channels.clear();
        poll_return_time = poller->Poll(POLL_TIME_MS, active_channels);
        for (auto& ch : active_channels)
        {
            ch->HandleEvent();
        }
        DoPendingFunctors();
    }

    LOG_TRACE << "EventLoop " << this << " stop looping";
    looping = false;
}

void EventLoop::Quit()
{
    quit = true;
    if (!IsInLoopThread())
    {
        WakeUp();
    }
}

void EventLoop::RunInLoop(const Functor& cb)
{
    if (IsInLoopThread())
    { 
        cb();
    }
    else
    {
        QueueInLoop(cb);
    }
}

void EventLoop::QueueInLoop(const Functor& cb)
{
    {
        std::lock_guard lock(mutex_);
        pending_functors.push_back(cb);
    }

    if (!IsInLoopThread() || calling_pending_functors)
    {
        WakeUp();
    }
}

TimerId EventLoop::RunAt(const TimeStamp& time, const TimerCallback& cb)
{
    using namespace std::chrono_literals;
    return timer_queue->AddTimer(cb, time, 0s);
}

TimerId EventLoop::RunAfter(const Duration& delay, const TimerCallback& cb)
{
    return RunAt(AddTime(TimeStamp::Now(), delay), cb);
}

TimerId EventLoop::RunEvery(const Duration& interval, const TimerCallback& cb)
{
    return timer_queue->AddTimer(cb, AddTime(TimeStamp::Now(), interval), interval);
}

void EventLoop::UpdateChannel(Channel* channel)
{
    assert(channel->OwnerLoop() == this);
    AssertInLoopThread();
    poller->UpdateChannel(channel);
}

void EventLoop::AssertInLoopThread()
{
    if (!IsInLoopThread())
    {
        AbortNotInLoopThread();
    }
}

bool EventLoop::IsInLoopThread() const
{
    return thread_id == Tid();
}

void EventLoop::AbortNotInLoopThread()
{
    LOG_FATAL << "EventLoop::AbortNotInLoopThread - EventLoop " << this
              << " was created in thread_id = " << Tid_64(thread_id)
              << ", current thread id = " << Tid_64();
}

void EventLoop::WakeUp()
{
    uint64_t one = 1;
    ssize_t n = ::write(wakeup_fd, &one, sizeof(one));
    if (n != sizeof(one))
    {
        LOG_ERROR << "EventLoop::WakeUp() writes " << n << " bytes instead of 8";
    }
}

void EventLoop::HandleRead()
{
    uint64_t one = 1;
    ssize_t n = ::read(wakeup_fd, &one, sizeof(one));
    if (n != sizeof(one))
    {
        LOG_ERROR << "EventLoop::HandleRead() reads " << n << " bytes instead of 8";
    }
}

void EventLoop::DoPendingFunctors()
{
    calling_pending_functors = true;

    std::vector<Functor> functors;
    {
        std::lock_guard lock(mutex_);
        functors.swap(pending_functors);
    }

    for (auto& f : functors)
    {
        f();
    }

    calling_pending_functors = false;
}
