#pragma once

#include <native/core/obsidian/include.h>

struct Utils
{
    static std::string read_file(const std::string& path);
    static void write_file(const std::string& path, const IsBinaryContainer auto& data)
    {
        std::ofstream file(path, std::ios::binary | std::ios::out | std::ios::trunc);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to write file: " + path);
        }

        using ValueType = std::ranges::range_value_t<decltype(data)>;
        size_t total_bytes = std::size(data) * sizeof(ValueType);
        file.write(reinterpret_cast<const char*>(std::data(data)), total_bytes);
    }
};