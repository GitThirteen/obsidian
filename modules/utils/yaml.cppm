module;
#include <native/macros.h>
#include <fstream>
#include <sstream>

// ===================== //
//  UTILS - YAML MODULE  //
// ===================== //

export module Obsidian.Utils.Yaml;

import std;
import Obsidian.Core.Result;

EXPORT(obsidian, yaml)

enum class NodeType { Null, String, Number, Boolean, Array, Object };

struct Node
{
    NodeType type = NodeType::Null;
    std::string string_val;
    double num_val = 0.0;
    bool bool_val = false;
    std::vector<Node> array_val;
    std::unordered_map<std::string, Node> object_val;

    auto as_string() const -> std::string { return string_val; }
    auto as_int() const -> int { return static_cast<int>(num_val); }
    auto as_float() const -> float { return static_cast<float>(num_val); }
    auto as_bool() const -> bool { return bool_val; }

    auto operator[](std::string_view key) const -> const Node&
    {
        static Node null_node;
        auto it = object_val.find(std::string(key));
        return (it != object_val.end()) ? it->second : null_node;
    }

    auto operator[](size_t index) const -> const Node&
    {
        static Node null_node;
        return (index < array_val.size()) ? array_val[index] : null_node;
    }

    auto has(std::string_view key) const -> bool
    {
        return object_val.contains(std::string(key));
    }
};

auto parse_value(std::string_view value) -> Node;

auto parse(std::string_view yaml_content) -> Result<Node>
{
    Node root{ NodeType::Object };
    std::vector<std::pair<int, Node*>> stack;
    stack.push_back({ -1, &root });

    std::istringstream stream{ std::string(yaml_content) };
    std::string line;

    while (std::getline(stream, line))
    {
        if (auto pos = line.find('#'); pos != std::string::npos) line = line.substr(0, pos);
        line.erase(line.find_last_not_of(" \n\r\t") + 1);
        if (line.empty()) continue;

        int indent = 0;
        while (indent < line.size() && line[indent] == ' ') indent++;
        std::string content = line.substr(indent);

        while (stack.size() > 1 && stack.back().first >= indent)
        {
            stack.pop_back();
        }

        Node* parent = stack.back().second;

        if (content.starts_with("- "))
        {
            if (parent->type != NodeType::Array) parent->type = NodeType::Array;
            std::string val_str = content.substr(2);

            if (val_str.empty())
            {
                parent->array_val.push_back(Node{ NodeType::Object });
                stack.push_back({ indent, &parent->array_val.back() });
            }
            else
            {
                parent->array_val.push_back(parse_value(val_str));
            }

            continue;
        }

        if (auto colon_pos = content.find(':'); colon_pos != std::string::npos)
        {
            std::string key = content.substr(0, colon_pos);
            std::string val_str = content.substr(colon_pos + 1);
            val_str.erase(0, val_str.find_first_not_of(" \t")); // trim leading

            if (parent->type != NodeType::Object) parent->type = NodeType::Object;

            if (val_str.empty())
            {
                parent->object_val[key] = Node{ NodeType::Object };
                stack.push_back({ indent, &parent->object_val[key] });
            }
            else
            {
                parent->object_val[key] = parse_value(val_str);
            }
        }
    }

    return root;
}

auto parse_value(std::string_view v) -> Node
{
    if (v.starts_with('"') && v.ends_with('"'))
    {
        return Node{ NodeType::String, std::string(v.substr(1, v.size() - 2)) };
    }

    if (v == "true" || v == "false")
    {
        return Node{ NodeType::Boolean, "", 0.0, v == "true" };
    }

    if (v.starts_with('[') && v.ends_with(']'))
    {
        Node arr{ NodeType::Array };
        std::string inner = std::string(v.substr(1, v.size() - 2));
        std::istringstream ss(inner);
        std::string item;

        while (std::getline(ss, item, ','))
        {
            item.erase(0, item.find_first_not_of(" \t"));
            item.erase(item.find_last_not_of(" \t") + 1);
            arr.array_val.push_back(parse_value(item));
        }

        return arr;
    }

    try
    {
        size_t idx;
        double num = std::stod(std::string(v), &idx);
        if (idx == v.size()) return Node{ NodeType::Number, "", num };
    }
    catch (...) {}

    return Node{ NodeType::String, std::string(v) };
}

auto parse_file(std::string_view filepath) -> Result<Node>
{
    std::ifstream file(filepath.data());
    if (!file.is_open())
    {
        return std::unexpected(std::format("Failed to open YAML config: {}", filepath));
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return parse(buffer.str());
}

EXPORT_END