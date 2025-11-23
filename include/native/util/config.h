#pragma once

#include <native/core/obsidian/include.h>

struct Config
{
    template <typename T, typename... Args>
    requires (IsNumeric<T> || IsLikeString<T>) && (... && IsLikeString<Args>)
    static T get(Args... values)
    {
        if (Config::data.empty())
        {
            Config::read_to_local();
        }

        ryml::Tree tree;
        try {
            tree = ryml::parse_in_place(c4::to_substr(Config::data));
        }
        catch (...)
        {
            throw std::runtime_error("Unable to parse buffered config data.");
        }

        try {
            ryml::NodeRef curr_ref = tree.rootref();

            for (const auto& value : {values...})
            {
                curr_ref = curr_ref[value];
            }

            T value;
            curr_ref >> value;
            return value;
        }
        catch (...)
        {
            throw std::runtime_error("Data access error.");
        }
    }

    static void read_to_local(std::string path = "./config.yaml");

private:
    static std::string data;
};