#pragma once
#include "EventLoop.h"
#include "datetime/TimeStamp.h"
#include <map>
#include <poll.h>
#include <vector>

class Channel;

class Poller : NonCopyable
{
public:
    using ChannelList = std::vector<Channel*>;

public:
    Poller(EventLoop* loop);

    TimeStamp Poll(int timeout_ms, ChannelList* active_channels);
    void UpdateChannel(Channel* channel);

    void AssertInLoopThread() { owner_loop->AssertInLoopThread(); }

private:
    void FillActiveChannels(int num_events, ChannelList* active_channels) const;

private:
    using PollFdList = std::vector<struct pollfd>;
    using ChannelMap = std::map<int, Channel*>;

    EventLoop* owner_loop;
    PollFdList poll_fds;
    ChannelMap channels;
};
