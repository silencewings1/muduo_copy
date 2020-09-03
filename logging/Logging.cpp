#include "Logging.h"
#include "thread/Thread.h"
#include <errno.h>
#include <iostream>

thread_local char t_errnobuf[512];
thread_local char t_time[32];
thread_local time_t t_lastSecond;

const char* strerror_tl(int savedErrno)
{
    return strerror_r(savedErrno, t_errnobuf, sizeof(t_errnobuf));
}

Logger::LogLevel InitLogLevel()
{
    if (::getenv("MUDUO_LOG_TRACE"))
        return Logger::LogLevel::TRACE;
    else
        return Logger::LogLevel::DEBUG;
}

Logger::LogLevel g_logLevel = InitLogLevel();

const char* LogLevelName[static_cast<int>(Logger::LogLevel::NUM_LOG_LEVELS)] = {
    "TRACE ",
    "DEBUG ",
    "INFO  ",
    "WARN  ",
    "ERROR ",
    "FATAL ",
};

Logger::OutputFun g_output = [](const char* msg, int len) {
    size_t n = fwrite(msg, 1, len, stdout);
    //FIXME check n
    (void)n;
};

Logger::FlushFun g_flush = []() { fflush(stdout); };

Logger::Impl::Impl(LogLevel level, int savedErrno, const char* file, int line)
    : time(TimeStamp::Now())
    , stream()
    , level(level)
    , line(line)
    , full_name(file)
    , base_name(nullptr)
{
    const char* path_sep_pos = strrchr(full_name, '/');
    base_name = (path_sep_pos != NULL) ? path_sep_pos + 1 : full_name;

    FormatTime();
    Fmt tid("%5d ", RawTid());
    assert(tid.Length() == 6);
    stream << T(tid.Data(), 6);
    stream << T(LogLevelName[static_cast<int>(level)], 6);
    if (savedErrno != 0)
    {
        stream << strerror_tl(savedErrno) << " (errno=" << savedErrno << ") ";
    }
}

void Logger::Impl::FormatTime()
{
    int64_t microSecondsSinceEpoch = time.MicroSecondsSinceEpoch();
    time_t seconds = static_cast<time_t>(microSecondsSinceEpoch / 1000000);
    int microseconds = static_cast<int>(microSecondsSinceEpoch % 1000000);
    if (seconds != t_lastSecond)
    {
        t_lastSecond = seconds;
        struct tm tm_time;
        ::gmtime_r(&seconds, &tm_time); // FIXME TimeZone::fromUtcTime

        int len = snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d",
                           tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                           tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
        assert(len == 17);
        (void)len;
    }
    Fmt us(".%06dZ ", microseconds);
    assert(us.Length() == 9);
    stream << T(t_time, 17) << T(us.Data(), 9);
}

void Logger::Impl::Finish()
{
    stream << " - " << base_name << ':' << line << '\n';
}

Logger::Logger(const char* file, int line)
    : impl(LogLevel::INFO, 0, file, line)
{
}

Logger::Logger(const char* file, int line, LogLevel level, const char* func)
    : impl(level, 0, file, line)
{
    impl.stream << func << ' ';
}

Logger::Logger(const char* file, int line, LogLevel level)
    : impl(level, 0, file, line)
{
}

Logger::Logger(const char* file, int line, bool toAbort)
    : impl(toAbort ? LogLevel::FATAL : LogLevel::ERROR, errno, file, line)
{
}

Logger::~Logger()
{
    impl.Finish();
    const LogStream::Buffer& buf(Stream().GetBuffer());
    g_output(buf.Data(), buf.Length());
    if (impl.level == LogLevel::FATAL)
    {
        g_flush();
        abort();
    }
}

Logger::LogLevel Logger::logLevel()
{
    return g_logLevel;
}

void Logger::SetLogLevel(Logger::LogLevel level)
{
    g_logLevel = level;
}

void Logger::SetOutput(OutputFun out)
{
    g_output = out;
}

void Logger::SetFlush(FlushFun flush)
{
    g_flush = flush;
}
