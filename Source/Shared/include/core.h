#pragma once

#include "assert.h"

using uint64 = unsigned long long;
using  int64 = signed long long;
using uint32 = unsigned int;
using  int32 = signed int;
using uint16 = unsigned short;
using  int16 = signed short;
using  uint8 = unsigned char;
using   int8 = signed char;

// Asserts if this path is ever hit.
#define assertNoEntry() assert(false);

constexpr const char* ENDLINE = "\n";

namespace debug
{
    void info(const char* format, ...);
    void warn(const char* format, ...);
    void error(const char* format, ...);
} // !debug
