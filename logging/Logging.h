#pragma once
#include "LogStream.h"
#include "datetime/TimeStamp.h"
#include <functional>
#include <memory>

class Logger
{
public:
    enum class LogLevel
    {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        NUM_LOG_LEVELS,
    };

    using OutputFun = std::function<void(const char*, int)>;
    using FlushFun = std::function<void()>;

public:
    Logger(const char* file, int line);
    Logger(const char* file, int line, LogLevel level);
    Logger(const char* file, int line, LogLevel level, const char* func);
    Logger(const char* file, int line, bool toAbort);
    ~Logger();

    LogStream& Stream() { return impl.stream; }

    static LogLevel logLevel();
    static void SetLogLevel(LogLevel level);
    static void SetOutput(OutputFun);
    static void SetFlush(FlushFun);

private:
    struct Impl
    {
        using LogLevel = Logger::LogLevel;

        Impl(LogLevel level, int old_errno, const char* file, int line);
        void FormatTime();
        void Finish();

        TimeStamp time;
        LogStream stream;
        LogLevel level;
        int line;

        const char* full_name;
        const char* base_name;
    };

    Impl impl;
};

#define LOG_TRACE                                      \
    if (Logger::logLevel() <= Logger::LogLevel::TRACE) \
    Logger(__FILE__, __LINE__, Logger::LogLevel::TRACE, __func__).Stream()
#define LOG_DEBUG                                      \
    if (Logger::logLevel() <= Logger::LogLevel::DEBUG) \
    Logger(__FILE__, __LINE__, Logger::LogLevel::DEBUG, __func__).Stream()
#define LOG_INFO                                      \
    if (Logger::logLevel() <= Logger::LogLevel::INFO) \
    Logger(__FILE__, __LINE__).Stream()
#define LOG_WARN Logger(__FILE__, __LINE__, Logger::LogLevel::WARN).Stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::LogLevel::ERROR).Stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::LogLevel::FATAL).Stream()
#define LOG_SYSERR Logger(__FILE__, __LINE__, false).Stream()
#define LOG_SYSFATAL Logger(__FILE__, __LINE__, true).Stream()

const char* strerror_tl(int savedErrno);

// Taken from glog/logging.h
//
// Check that the input is non NULL.  This very useful in constructor
// initializer lists.

#define CHECK_NOTNULL(val) \
    ::CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

// A small helper for CHECK_NOTNULL().
template <typename T>
T* CheckNotNull(const char* file, int line, const char* names, T* ptr)
{
    if (ptr == nullptr)
    {
        Logger(file, line, Logger::LogLevel::FATAL).Stream() << names;
    }
    return ptr;
}

template <typename To, typename From>
inline To implicit_cast(From const& f)
{
    return f;
}