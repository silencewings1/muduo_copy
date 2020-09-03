#include "Thread.h"
#include <sstream>
#include <sys/syscall.h>
#include <unistd.h>

namespace
{

thread_local pid_t cached_tid = 0;

pid_t GetTidCall()
{
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

} // namespace

pid_t RawTid()
{
    if (cached_tid == 0)
    {
        cached_tid = GetTidCall();
    }

    return cached_tid;
}

ThreadID Tid()
{
    return std::this_thread::get_id();
}

uint64_t Tid_64(const ThreadID& id)
{
    std::stringstream ss;
    ss << id;
    return std::stoull(ss.str());
}

uint64_t Tid_64()
{
    return Tid_64(std::this_thread::get_id());
}

bool IsMainThread()
{
    return ::getpid() == RawTid();
}