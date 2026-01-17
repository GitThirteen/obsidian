#include <native/utils/io.h>

auto ObsidianIO::read_file(const std::string& path) -> std::string
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}