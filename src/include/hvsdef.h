#pragma once
#include <exception>
#include <iostream>
#include "common/JsonSerializer.h"

// LATEX editor: overleaf
// cmake auto adds -DNDEBUG to flags if compiled in release mode
#ifndef NDEBUG
#   define ASSERT(condition, message) \
    do { \
        if (! (condition)) { \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__ \
                      << " line " << __LINE__ << ": " << message << std::endl; \
            std::terminate(); \
        } \
    } while (false)
#else
#   define ASSERT(condition, message) do { } while (false)
#endif
