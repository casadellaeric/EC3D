#pragma once

#include "pch.hpp"

#ifndef NDEBUG
    #define EC_ASSERTIONS_ENABLED
#endif

#ifdef _MSC_VER
    #define DEBUG_BREAK __debugbreak()
#else
    #define DEBUG_BREAK
#endif

#ifdef EC_ASSERTIONS_ENABLED
    #define EC_ASSERT(expression)                                 \
        if (expression) [[likely]] {                              \
        } else {                                                  \
            EC_LOG_CRITICAL("Assertion failed: {}", #expression); \
            DEBUG_BREAK;                                          \
        }
#else
    #define EC_ASSERT(expression)
#endif
