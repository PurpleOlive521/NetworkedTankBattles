#include "core.h"
#include <cassert>
#include <string_view>
#include <WS2tcpip.h>

namespace debug
{
    void info(const char* format, ...)
    {
        char msg[1024] = {};
        va_list args;
        va_start(args, format);
        int len = vsprintf_s(msg, format, args);
        va_end(args);
        printf("INF: %s\n", msg);
    }

    void warn(const char* format, ...)
    {
        char msg[1024] = {};
        va_list args;
        va_start(args, format);
        int len = vsprintf_s(msg, format, args);
        va_end(args);
        printf("WRN: %s\n", msg);
    }

    void error(const char* format, ...)
    {
        char msg[1024] = {};
        va_list args;
        va_start(args, format);
        int len = vsprintf_s(msg, format, args);
        va_end(args);
        printf("ERR: %s\n", msg);

        assert(false);
    }
} // !debug