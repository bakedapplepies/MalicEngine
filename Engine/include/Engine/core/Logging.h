#pragma once

#include <iostream>
#include <thread>
#include <fmt/format.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Engine/core/Defines.h"

extern std::ostream& operator<<(std::ostream& stream, const glm::vec2& vec2);
extern std::ostream& operator<<(std::ostream& stream, const glm::vec3& vec3);
extern std::ostream& operator<<(std::ostream& stream, const glm::mat3& mat3);
extern std::ostream& operator<<(std::ostream& stream, const glm::mat4& mat4);

template <>
class fmt::formatter<glm::vec2>
{
public:
    constexpr auto parse (format_parse_context& ctx) { return ctx.begin(); }

    template <typename Context>
    constexpr auto format (const glm::vec2& vec2, Context& ctx) const
    {
        return format_to(ctx.out(), "[{}, {}]", vec2.x, vec2.y);
    }
};

template <>
class fmt::formatter<glm::vec3>
{
public:
    constexpr auto parse (format_parse_context& ctx) { return ctx.begin(); }

    template <typename Context>
    constexpr auto format (const glm::vec3& vec3, Context& ctx) const
    {
        return format_to(ctx.out(), "[{}, {}, {}]", vec3.x, vec3.y, vec3.z);
    }
};

template <>
class fmt::formatter<glm::mat3>
{
public:
    constexpr auto parse (format_parse_context& ctx) { return ctx.begin(); }

    template <typename Context>
    constexpr auto format (const glm::mat3& mat3, Context& ctx) const
    {
        return format_to(ctx.out(), "{}", glm::to_string(mat3));
    }
};

template <>
class fmt::formatter<glm::mat4>
{
public:
    constexpr auto parse (format_parse_context& ctx) { return ctx.begin(); }

    template <typename Context>
    constexpr auto format (const glm::mat4& mat4, Context& ctx) const
    {
        return format_to(ctx.out(), "{}", glm::to_string(mat4));
    }
};

template <>
class fmt::formatter<std::thread::id>
{
public:
    constexpr auto parse (format_parse_context& ctx) { return ctx.begin(); }

    template <typename Context>
    constexpr auto format (const std::thread::id& thread_id, Context& ctx) const
    {
        return format_to(ctx.out(), "{}", std::hash<std::thread::id>{}(thread_id));
    }
};

MLC_NAMESPACE_START

enum class LogLevel: uint32_t
{
    LVL_FATAL = 0,
    LVL_ERROR,
    LVL_WARN,
    LVL_INFO,
    LVL_DEBUG
};

static const char* LOG_LEVELS[] = {
    "FATAL",
    "ERROR",
    "WARN",
    "INFO",
    "DEBUG"
};

template <typename... Args>
static void __Log(
    LogLevel log_level,
    const char* file,
    size_t line,
    fmt::format_string<Args...> format_str,
    Args&&... args
) {
    fmt::print("[{} ~{}:{}] {}\n",
        LOG_LEVELS[static_cast<uint32_t>(log_level)],
        file,
        line,
        fmt::format(format_str, std::forward<Args>(args)...)
    );
}

MLC_NAMESPACE_END

// Adjust Logging settings here
#define MLC_LOG_FATAL_ENABLED 1
#define MLC_LOG_ERROR_ENABLED 1
#define MLC_LOG_WARN_ENABLED  1
#define MLC_LOG_INFO_ENABLED  1

#if MLC_RELEASE
#   define MLC_LOG_DEBUG_ENABLED 0
#else
#   define MLC_LOG_DEBUG_ENABLED 1
#endif

#if MLC_LOG_FATAL_ENABLED == 1
#   define MLC_FATAL(...) \
        Malic::__Log(LogLevel::LVL_FATAL, __FILE__, __LINE__, __VA_ARGS__)
#else
#   define MLC_FATAL(...)
#endif

#if MLC_LOG_ERROR_ENABLED == 1
#   define MLC_ERROR(...) \
        Malic::__Log(LogLevel::LVL_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#else
#   define MLC_ERROR(...)
#endif

#if MLC_LOG_WARN_ENABLED == 1
#   define MLC_WARN(...) \
        Malic::__Log(LogLevel::LVL_WARN, __FILE__, __LINE__, __VA_ARGS__)
#else
#   define MLC_WARN(...)
#endif

#if MLC_LOG_INFO_ENABLED == 1
#   define MLC_INFO(...) \
        Malic::__Log(LogLevel::LVL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#else
#   define MLC_INFO(...)
#endif

#if MLC_LOG_DEBUG_ENABLED == 1
#   define MLC_DEBUG(...) \
        Malic::__Log(LogLevel::LVL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#else
#   define MLC_DEBUG(...)
#endif