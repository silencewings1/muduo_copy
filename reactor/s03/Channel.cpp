#include "Channel.h"
#include "EventLoop.h"
#include "logging/Logging.h"
#include <poll.h>

namespace
{

constexpr int NONE_EVENT = 0;
constexpr int READ_EVENT = POLLIN | POLLPRI;
constexpr int WRITE_EVENT = POLLOUT;

} // namespace

Channel::Channel(EventLoop* loop, int fd)
    : loop(loop)
    , fd_(fd)
    , events_(0)
    , revents_(0)
    , index(-1)
{
}

bool Channel::IsNoneEvent() const
{
    return events_ == NONE_EVENT;
}

void Channel::EnableReading()
{
    events_ |= READ_EVENT;
    update();
}

void Channel::update()
{
    loop->updateChannel(this);
}

void Channel::HandleEvent()
{
    if (revents_ & POLLNVAL)
        LOG_WARN << "Channel::handle_event() POLLNVAL";

    if (revents_ & (POLLERR | POLLNVAL))
        if (error_cb)
            error_cb();

    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))
        if (read_cb)
            read_cb();

    if (revents_ & POLLOUT)
        if (write_cb)
            write_cb();
}
