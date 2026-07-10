export module Obsidian.Utils:IO;
import std;
import Obsidian.Core;

export namespace obsidian::io 
{
    auto read_file(std::string_view path) -> Result<std::string> 
    {
        std::ifstream file(path.data(), std::ios::in | std::ios::binary);
        if (!file.is_open())
        {
            return std::unexpected(std::format("Failed to open file: {}", path));
        }

        std::string content;
        file.seekg(0, std::ios::end);
        content.resize(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(content.data(), content.size());
        
        return content;
    }

    template <typename T>
    requires std::ranges::contiguous_range<T> && std::ranges::sized_range<T>
    auto write_file(std::string_view path, const T& data) -> Result<void> 
    {
        std::ofstream file(path.data(), std::ios::binary | std::ios::out | std::ios::trunc);
        if (!file.is_open())
        {
            return std::unexpected(std::format("Failed to write to file: {}", path));
        }

        using ValueType = std::ranges::range_value_t<T>;
        size_t total_bytes = std::size(data) * sizeof(ValueType);
        file.write(reinterpret_cast<const char*>(std::data(data)), total_bytes);
        
        return {};
    }
}