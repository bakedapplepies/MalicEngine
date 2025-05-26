#pragma once

#include <fmt/format.h>

// Taken from https://github.com/travisvroman/kohi/blob/main/kohi.core/src/debug/kassert.h
// Try via __has_builtin first.
#if defined(__has_builtin) && !defined(__ibmxl__)
#    if __has_builtin(__builtin_debugtrap)
#        define MLC_DEBUG_BREAK() __builtin_debugtrap()
#    elif __has_builtin(__debugbreak)
#        define MLC_DEBUG_BREAK() __debugbreak()
#    endif
#endif

// If not setup, try the old way.
#if !defined(MLC_DEBUG_BREAK)
#    if defined(__clang__) || defined(__gcc__)
#        define MLC_DEBUG_BREAK() __builtin_trap()
#    elif defined(_MSC_VER)
#        include <intrin.h>
#        define MLC_DEBUG_BREAK() __debugbreak()
#    else
// Fall back to x86/x86_64
#        define MLC_DEBUG_BREAK() asm { int 3 }
#    endif
#endif

#ifndef MLC_RELEASE
#   define MLC_ASSERT(expr, msg)                                                        \
        {                                                                               \
            if (expr) {}                                                                \
            else {                                                                      \
                fmt::print("[ASSERTION FAILED | {}:{}] {}", __FILE__, __LINE__, msg);   \
                MLC_DEBUG_BREAK();                                                      \
            }                                                                           \
        }                                   
#else
#   define MLC_ASSERT(expr, msg)
#endif