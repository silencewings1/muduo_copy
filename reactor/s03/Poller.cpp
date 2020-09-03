#include "Poller.h"
#include "Channel.h"
#include "logging/Logging.h"
#include <assert.h>
#include <poll.h>

// #include <iostream>

Poller::Poller(EventLoop* loop)
    : owner_loop(loop)
{
}

TimeStamp Poller::Poll(int timeout_ms, ChannelList* active_channels)
{
    int nums = ::poll(poll_fds.data(), poll_fds.size(), timeout_ms);
    auto now = TimeStamp::Now();
    if (nums > 0)
    {
        // std::cout << nums << " events happended\n";
        LOG_TRACE << nums << " events happended";
        FillActiveChannels(nums, active_channels);
    }
    else if (nums == 0)
    {
        LOG_TRACE << " nothing happended";
    }
    else
    {
        LOG_SYSERR << "Poller::Poll()";
    }

    return now;
}

void Poller::FillActiveChannels(int num_events, ChannelList* active_channels) const
{
    for (const auto& poll_fd : poll_fds)
    {
        if (num_events <= 0)
            break;

        if (poll_fd.revents > 0)
        {
            --num_events;

            const auto ch = channels.find(poll_fd.fd);
            assert(ch != channels.end());
            auto channel = ch->second;
            assert(channel->fd() == poll_fd.fd);
            channel->set_revents(poll_fd.revents);

            active_channels->push_back(channel);
        }
    }
}

void Poller::UpdateChannel(Channel* channel)
{
    AssertInLoopThread();
    LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();

    if (channel->Index() < 0)
    {
        // a new one, add to poll_fds
        assert(channels.find(channel->fd()) == channels.end());

        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;

        poll_fds.push_back(pfd);

        int idx = static_cast<int>(poll_fds.size()) - 1;
        channel->SetIndex(idx);
        channels[pfd.fd] = channel;
    }
    else
    {
        // update existing one
        assert(channels.find(channel->fd()) != channels.end());
        assert(channels[channel->fd()] == channel);

        int idx = channel->Index();
        assert(0 <= idx && idx < static_cast<int>(poll_fds.size()));

        struct pollfd& pfd = poll_fds[idx];
        assert(pfd.fd == channel->fd() || pfd.fd == -1);

        pfd.fd = channel->IsNoneEvent() ? -1 : pfd.fd;
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
    }
}
