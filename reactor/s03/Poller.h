#pragma once
#include "EventLoop.h"
#include "datetime/TimeStamp.h"
#include <map>
#include <poll.h>
#include <vector>

struct pollfd;

class Channel;

class Poller : NonCopyable
{
public:
    using ChannelList = std::vector<Channel*>;

public:
    Poller(EventLoop* loop);

    TimeStamp poll(int timeoutMs, ChannelList* activeChannels);
    void UpdateChannel(Channel* channel);

    void AssertInLoopThread() { ownerLoop_->AssertInLoopThread(); }

private:
    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

private:
    using PollFdList = std::vector<struct pollfd>;
    using ChannelMap = std::map<int, Channel*>;

    EventLoop* ownerLoop_;
    PollFdList pollfds_;
    ChannelMap channels_;
};
