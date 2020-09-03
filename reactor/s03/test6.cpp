#include "EventLoop.h"
#include "EventLoopThread.h"
#include <chrono>
#include <iostream>
#include <unistd.h>
using namespace std::chrono_literals;

void runInThread()
{
    std::cout << "Fun: pid=" << getpid() << ", tid=" << Tid() << std::endl;
}

int main()
{
    std::cout << "main: pid=" << getpid() << ", tid=" << Tid() << std::endl;

    {
        EventLoopThread loopThread;
        EventLoop* loop = loopThread.startLoop();

        loop->runInLoop(runInThread);
        std::this_thread::sleep_for(1s);

        loop->runAfter(2s, runInThread);
        std::this_thread::sleep_for(3s);

        loop->quit();
    }

    std::cout << "exit main\n";

    return 0;
}
