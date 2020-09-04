#include "EventLoop.h"
#include <chrono>
#include <stdio.h>
#include <unistd.h>
using namespace std::chrono_literals;

EventLoop* g_loop;
int g_flag = 0;

void run4()
{
    printf("run4(): pid = %d, flag = %d\n", getpid(), g_flag);
    g_loop->Quit();
}

void run3()
{
    printf("run3(): pid = %d, flag = %d\n", getpid(), g_flag);
    g_loop->RunAfter(3s, run4);
    g_flag = 3;
}

void run2()
{
    printf("run2(): pid = %d, flag = %d\n", getpid(), g_flag);
    g_loop->QueueInLoop(run3);
}

void run1()
{
    g_flag = 1;
    printf("run1(): pid = %d, flag = %d\n", getpid(), g_flag);
    g_loop->RunInLoop(run2);
    g_flag = 2;
}

int main()
{
    printf("main(): pid = %d, flag = %d\n", getpid(), g_flag);

    EventLoop loop;
    g_loop = &loop;

    loop.RunAfter(2s, run1);
    loop.Loop();
    printf("main(): pid = %d, flag = %d\n", getpid(), g_flag);

    return 0;
}
