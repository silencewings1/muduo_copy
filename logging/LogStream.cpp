#include "LogStream.h"
#include "thread/Thread.h"
#include <algorithm>

namespace
{

const char digits[] = "9876543210123456789";
const char* zero = digits + 9;
static_assert(sizeof(digits) == 20);

const char digitsHex[] = "0123456789abcdef";
static_assert(sizeof(digitsHex) == 17);

// Efficient Integer to String Conversions, by Matthew Wilson.
template <typename T>
size_t convert(char buf[], T value)
{
    T i = value;
    char* p = buf;

    do
    {
        int lsd = i % 10;
        i /= 10;
        *p++ = zero[lsd];
    } while (i != 0);

    if (value < 0)
    {
        *p++ = '-';
    }
    *p = '\0';
    std::reverse(buf, p);

    return p - buf;
}

size_t convertHex(char buf[], uintptr_t value)
{
    uintptr_t i = value;
    char* p = buf;

    do
    {
        int lsd = i % 16;
        i /= 16;
        *p++ = digitsHex[lsd];
    } while (i != 0);

    *p = '\0';
    std::reverse(buf, p);

    return p - buf;
}

} // namespace

template class detail::FixedBuffer<detail::SMALL_BUFFER>;
template class detail::FixedBuffer<detail::LARGE_BUFFER>;

template <typename T>
void LogStream::FormatInteger(T v)
{
    if (buffer.Avail() >= MAX_NUMERIC_SIZE)
    {
        size_t len = convert(buffer.Current(), v);
        buffer.Add(len);
    }
}

LogStream& LogStream::operator<<(short v)
{
    *this << static_cast<int>(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned short v)
{
    *this << static_cast<unsigned int>(v);
    return *this;
}

LogStream& LogStream::operator<<(int v)
{
    FormatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned int v)
{
    FormatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long v)
{
    FormatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long v)
{
    FormatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long long v)
{
    FormatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long long v)
{
    FormatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(const void* p)
{
    uintptr_t v = reinterpret_cast<uintptr_t>(p);
    if (buffer.Avail() >= MAX_NUMERIC_SIZE)
    {
        char* buf = buffer.Current();
        buf[0] = '0';
        buf[1] = 'x';
        size_t len = convertHex(buf + 2, v);
        buffer.Add(len + 2);
    }
    return *this;
}

// FIXME: replace this with Grisu3 by Florian Loitsch.
LogStream& LogStream::operator<<(double v)
{
    if (buffer.Avail() >= MAX_NUMERIC_SIZE)
    {
        int len = snprintf(buffer.Current(), MAX_NUMERIC_SIZE, "%.12g", v);
        buffer.Add(len);
    }
    return *this;
}

/////////////////////////
template <typename T>
Fmt::Fmt(const char* fmt, T val)
{
    static_assert(std::is_arithmetic_v<T> == true);

    len = snprintf(buf, sizeof(buf), fmt, val);
    assert(static_cast<size_t>(len) < sizeof(buf));
}

template Fmt::Fmt(const char* fmt, char);

template Fmt::Fmt(const char* fmt, short);
template Fmt::Fmt(const char* fmt, unsigned short);
template Fmt::Fmt(const char* fmt, int);
template Fmt::Fmt(const char* fmt, unsigned int);
template Fmt::Fmt(const char* fmt, long);
template Fmt::Fmt(const char* fmt, unsigned long);
template Fmt::Fmt(const char* fmt, long long);
template Fmt::Fmt(const char* fmt, unsigned long long);

template Fmt::Fmt(const char* fmt, float);
template Fmt::Fmt(const char* fmt, double);
