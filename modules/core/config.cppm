module;
#include <native/macros.h>

export module Obsidian.Core.Config;

import std;
import Obsidian.Core.Result;
import Obsidian.Core.Logger;
import Obsidian.Utils.Yaml;

EXPORT(obsidian)

class Config
{
public:
    template <typename T, typename... Keys>
	static auto get(Keys... keys) -> Result<T>
    {
        fetch_config();

        std::array<std::string_view, sizeof...(Keys)> path{ keys... };
        const yaml::Node* current_node = &s_root;

        for (const auto& key : path)
        {
            if (!current_node->has(key))
            {
                return std::unexpected(std::format("Configuration key '{}' not found.", key));
            }

            current_node = &(*current_node)[key];
        }

        const yaml::Node& node = *current_node;

        if constexpr (std::is_same_v<T, int>) return node.as_int();
        else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) return node.as_float();
        else if constexpr (std::is_same_v<T, bool>) return node.as_bool();
        else if constexpr (std::is_same_v<T, std::string>) return node.as_string();
        else if constexpr (std::is_same_v<T, std::vector<std::string>>)
        {
            std::vector<std::string> vec;
            if (node.type != yaml::NodeType::Array)
            {
                return std::unexpected(std::format("Key '{}' is not an array.", path.back()));
            }

            for (const auto& item : node.array_val)
            {
                vec.push_back(item.as_string());
            }
            return vec;
        }
        else
        {
            static_assert(std::is_void_v<T>, "Unsupported config type requested!");
        }
    }

private:
    static yaml::Node s_root;
    static bool s_loaded;

    static void fetch_config()
    {
        if (s_loaded) return;

        if (auto res = yaml::parse_file("config.yaml"); res)
        {
            s_root = std::move(res.inner.value());
        }
        else
        {
            log::warn("Failed to load config.yaml! Falling back to empty configuration. Error: {}", res.error());
            s_root = yaml::Node{ yaml::NodeType::Object };
        }

        s_loaded = true;
    }
};

yaml::Node Config::s_root;
bool Config::s_loaded = false;

EXPORT_END