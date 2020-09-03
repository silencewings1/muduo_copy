#pragma once
#include "base/NonCopyable.h"
#include <assert.h>
#include <string.h>
#include <string>

namespace detail
{

constexpr int SMALL_BUFFER = 4000;
constexpr int LARGE_BUFFER = 4000 * 1000;

template <int Size>
class FixedBuffer : NonCopyable
{
public:
    FixedBuffer()
        : cur(data)
    {
    }

    void Append(const char* buf, int len)
    {
        if (Avail() > len)
        {
            memcpy(cur, buf, len);
            cur += len;
        }
    }

    const char* Data() const
    {
        return data;
    }

    int Length() const
    {
        return cur - data;
    }

    char* Current() const
    {
        return cur;
    }

    int Avail() const
    {
        return static_cast<int>(End() - cur);
    }

    void Add(size_t len)
    {
        cur += len;
    }

    void Reset()
    {
        cur = data;
    }

    void Bzero()
    {
        ::bzero(data, sizeof(data));
    }

private:
    const char* End() const
    {
        return data + sizeof(data);
    }

private:
    // TODO: cookie
    char data[Size];
    char* cur;
};

} // namespace detail

struct T
{
    T(const char* str, int len)
        : str(str)
        , len(len)
    {
        // assert(strlen(str) == len);
    }

    const char* str;
    const size_t len;
};

class LogStream : NonCopyable
{
public:
    using Buffer = detail::FixedBuffer<detail::SMALL_BUFFER>;

public:
    LogStream& operator<<(bool v)
    {
        buffer.Append(v ? "1" : "0", 1);
        return *this;
    }

    LogStream& operator<<(short);
    LogStream& operator<<(unsigned short);
    LogStream& operator<<(int);
    LogStream& operator<<(unsigned int);
    LogStream& operator<<(long);
    LogStream& operator<<(unsigned long);
    LogStream& operator<<(long long);
    LogStream& operator<<(unsigned long long);

    LogStream& operator<<(const void*);

    LogStream& operator<<(float v)
    {
        *this << static_cast<double>(v);
        return *this;
    }
    LogStream& operator<<(double);

    LogStream& operator<<(char v)
    {
        buffer.Append(&v, 1);
        return *this;
    }

    LogStream& operator<<(const char* v)
    {
        buffer.Append(v, strlen(v));
        return *this;
    }

    LogStream& operator<<(const T& v)
    {
        buffer.Append(v.str, v.len);
        return *this;
    }

    LogStream& operator<<(const std::string& v) // FIXME: StringPiece
    {
        buffer.Append(v.c_str(), v.size());
        return *this;
    }

    void Append(const char* data, int len) { buffer.Append(data, len); }
    const Buffer& GetBuffer() const { return buffer; }
    void ResetBuffer() { buffer.Reset(); }

private:
    template <typename T>
    void FormatInteger(T);

private:
    Buffer buffer;
    static constexpr int MAX_NUMERIC_SIZE = 32;
};

class Fmt
{
public:
    template <typename T>
    Fmt(const char* fmt, T val);

    const char* Data() const { return buf; }
    int Length() const { return len; }

private:
    char buf[32];
    int len;
};

/////////////////////////////////////
inline LogStream& operator<<(LogStream& s, const Fmt& fmt)
{
    s.Append(fmt.Data(), fmt.Length());
    return s;
}