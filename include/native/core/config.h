#pragma once

#include <native/core/include.h>

struct ShardConfig
{
    struct Attachment
    {
        std::string type;
        std::string load;
        std::string store;
        std::string layout;
    };

    std::string name;
    std::string target;
    std::array<float, 4u> clear_color = { 0.0f, 0.0f, 0.0f, 0.0f };
    std::vector<Attachment> attachments;
    std::vector<std::string> pipelines;
};

namespace c4
{
    namespace yml
    {
        inline bool read(c4::yml::ConstNodeRef const& n, ShardConfig::Attachment* v)
        {
            if (!n.is_map()) return false;

            if (n.has_child("type")) n["type"] >> v->type;
            if (n.has_child("load")) n["load"] >> v->load;
            if (n.has_child("store")) n["store"] >> v->store;
            if (n.has_child("layout")) n["layout"] >> v->layout;

            return true;
        }

        inline bool read(c4::yml::ConstNodeRef const& n, ShardConfig* v)
        {
            if (!n.is_map()) return false;

            if (n.has_child("name")) n["name"] >> v->name;
            if (n.has_child("target")) n["target"] >> v->target;
            if (n.has_child("attachments")) n["attachments"] >> v->attachments;
            if (n.has_child("pipelines")) n["pipelines"] >> v->pipelines;
            if (n.has_child("clear_color")) n["clear_color"] >> v->clear_color;

            return true;
        }

        template <typename T, size_t N>
        inline bool read(c4::yml::ConstNodeRef const& n, std::array<T, N>* out)
        {
            if (!n.is_seq()) return false;

            size_t i = 0;
            for (const auto& child : n.children())
            {
                if (i >= N) break;
                child >> (*out)[i];
                ++i;
            }
            return true;
        }
    }
}

struct Config
{
    template <typename T, typename... Args>
    requires (IsNumeric<T> || IsLikeString<T> || IsVector<T>) && (... && IsLikeString<Args>)
    static auto get(Args... values) -> T
    {
        if (Config::data.empty())
        {
            Config::read_to_local();
        }

        ryml::Tree tree;
        try
        {
            tree = ryml::parse_in_place(c4::to_substr(Config::data));
        }
        catch (...)
        {
            throw std::runtime_error("Unable to parse buffered config data.");
        }

        try
        {
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

    template <typename... Args>
    requires (... && IsLikeString<Args>)
	static auto exists(Args... keys) -> bool
    {
        if (Config::data.empty())
        {
            return false;
        }

        ryml::Tree tree;
        try
        {
            tree = ryml::parse_in_place(c4::to_substr(Config::data));
        }
        catch (...)
        {
            return false;
        }

        ryml::NodeRef curr = tree.rootref();
        for (const auto& key : { keys... })
        {
            if (curr.val_is_null() || !curr.has_children() || !curr.has_child(key))
            {
                return false;
            }

            curr = curr[key];
        }

        return true;
    }

    template <typename... Args>
        requires (... && IsLikeString<Args>)
    static auto has_values_for(Args... keys) -> bool
    {
        if (Config::data.empty())
        {
            return false;
        }

        ryml::Tree tree;
        try
        {
            tree = ryml::parse_in_place(c4::to_substr(Config::data));
        }
        catch (...)
        {
            return false;
        }

        ryml::NodeRef curr = tree.rootref();
        for (const auto& key : { keys... })
        {
            if (!curr.has_child(key))
            {
                return false;
            }

            curr = curr[key];
        }

        return curr.has_children();
    }

    static auto read_to_local(std::string path = "./config.yaml") -> void;

private:
    static std::string data;
};