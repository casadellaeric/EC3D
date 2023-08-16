#pragma once

#include "pch.hpp"

#ifndef NDEBUG
    #define KL_ASSERTIONS_ENABLED
#endif

#ifdef _MSC_VER
    #define DEBUG_BREAK __debugbreak()
#else
    #define DEBUG_BREAK
#endif

#ifdef KL_ASSERTIONS_ENABLED
    #define KL_ASSERT(expression)                                 \
        if (expression) [[likely]] {                              \
        } else {                                                  \
            KL_LOG_CRITICAL("Assertion failed: {}", #expression); \
            DEBUG_BREAK;                                          \
        }
#else
    #define KL_ASSERT(expression)
#endif