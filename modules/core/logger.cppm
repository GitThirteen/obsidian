module;
#include <cstdlib>
#include <native/macros.h>

// =========================== //
//    CORE - LOGGER MODULE     //
// =========================== //

export module Obsidian.Core:Logger;
import std;

EXPORT(obsidian, log)

enum class Level { Debug, Info, Warn, Error, Fatal };

inline Level current_level = Level::Debug;

template <typename... Args>
auto print(Level level, std::string_view prefix, std::format_string<Args...> fmt, Args&&... args) -> void
{
    if (level < current_level) return;
    
    std::string message = std::format(fmt, std::forward<Args>(args)...);
    
    auto now = std::chrono::system_clock::now();
    std::string time_str = std::format("{:%H:%M:%S}", now);

    auto& out = (level >= Level::Error) ? std::cerr : std::cout;
    out << std::format("[{}] [{}] {}\n", time_str, prefix, message);
}

template <typename... Args>
auto debug(std::format_string<Args...> fmt, Args&&... args) -> void
{
    print(Level::Debug, "DEBUG", fmt, std::forward<Args>(args)...);
}

template <typename... Args>
auto info(std::format_string<Args...> fmt, Args&&... args) -> void
{
    print(Level::Info, "INFO", fmt, std::forward<Args>(args)...);
}

template <typename... Args>
auto warn(std::format_string<Args...> fmt, Args&&... args) -> void
{
    print(Level::Warn, "WARN", fmt, std::forward<Args>(args)...);
}

template <typename... Args>
auto error(std::format_string<Args...> fmt, Args&&... args) -> void
{
    print(Level::Error, "ERROR", fmt, std::forward<Args>(args)...);
}

template <typename... Args>
auto fatal(std::format_string<Args...> fmt, Args&&... args) -> void
{
    print(Level::Fatal, "FATAL", fmt, std::forward<Args>(args)...);
    std::exit(EXIT_FAILURE);
}

EXPORT_END