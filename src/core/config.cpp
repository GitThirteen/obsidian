#include <native/core/config.h>

std::string Config::data;

void Config::read_to_local(std::string path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        throw std::runtime_error("Unable to open config located at " + path);
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size))
    {
        throw std::runtime_error("Unable to read config located at " + path);
    }

    Config::data = std::string(buffer.begin(), buffer.end());
}
