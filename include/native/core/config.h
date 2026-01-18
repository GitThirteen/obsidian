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
    std::string options;
    std::array<float, 4u> clear_color = { 0.0f, 0.0f, 0.0f, 0.0f };
    std::vector<Attachment> attachments;
    std::vector<std::string> pipelines;
};

struct LightConfig
{
    struct Data
    {
        std::array<float, 3u> color = { 0.0f, 0.0f, 0.0f };
        std::array<float, 3u> direction = { 0.0f, 0.0f, 0.0f };
        std::array<float, 3u> position = { 0.0f, 0.0f, 0.0f };
        float intensity = 0.0f;
        float radius = 0.0f;
        float inner_cutoff = 0.0f;
        float outer_cutoff = 0.0f;
        float falloff = 0.0f;
    };

    std::string name;
    std::string type;
    std::vector<std::string> pipelines;
    Data data;
};

struct MaterialConfig
{
    struct Textures
    {
        std::string albedo;
        std::string roughness;
        std::string normal;
        std::string ao;
    };

    std::string name;
    std::array<float, 4> base_color = { 1.0f, 1.0f, 1.0f, 1.0f };
    float metallic = 0.0f;
    float roughness = 1.0f;
    Textures textures;
};

struct SceneElementConfig
{
    struct Data
    {
        // --- Common / Transform ---
        std::array<float, 3> position = { 0.0f, 0.0f, 0.0f };
        std::array<float, 3> rotate = { 0.0f, 0.0f, 0.0f };
        std::array<float, 3> scale = { 1.0f, 1.0f, 1.0f };

        // --- Mesh / Object ---
        std::string file;
        std::string material;

        // --- Volume ---
        size_t skip_bytes = 0;
        std::array<int, 3> dimensions = { 0, 0, 0 };
        std::string voxel_type; // "uint8", "uint16", etc.

        // --- Camera ---
        std::array<float, 3> target = { 0.0f, 0.0f, 0.0f };
    };

    std::string name;
    std::string type;
    std::vector<std::string> pipelines;
    Data data;
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
            if (n.has_child("options")) n["options"] >> v->options;

            return true;
        }

        template <typename T, size_t N>
        inline bool read(c4::yml::ConstNodeRef const& n, std::array<T, N>* v)
        {
            if (!n.is_seq()) return false;

            size_t i = 0;
            for (const auto& child : n.children())
            {
                if (i >= N) break;
                child >> (*v)[i];
                ++i;
            }

            return true;
        }

        inline bool read(c4::yml::ConstNodeRef const& n, MaterialConfig::Textures* v)
        {
            if (!n.is_map()) return false;

            if (n.has_child("albedo"))    n["albedo"] >> v->albedo;
            if (n.has_child("roughness")) n["roughness"] >> v->roughness;
            if (n.has_child("normal"))    n["normal"] >> v->normal;
            if (n.has_child("ao"))        n["ao"] >> v->ao;

            return true;
        }

        inline bool read(c4::yml::ConstNodeRef const& n, MaterialConfig* v)
        {
            if (!n.is_map()) return false;

            if (n.has_child("name"))       n["name"] >> v->name;
            if (n.has_child("base_color")) n["base_color"] >> v->base_color;
            if (n.has_child("metallic"))   n["metallic"] >> v->metallic;
            if (n.has_child("roughness"))  n["roughness"] >> v->roughness;
            if (n.has_child("textures"))   n["textures"] >> v->textures;

            return true;
        }

        inline bool read(c4::yml::ConstNodeRef const& n, SceneElementConfig::Data* v)
        {
            if (!n.is_map()) return false;

            // Common
            if (n.has_child("file"))     n["file"] >> v->file;
            if (n.has_child("position")) n["position"] >> v->position;
            if (n.has_child("rotate"))   n["rotate"] >> v->rotate;
            if (n.has_child("scale"))    n["scale"] >> v->scale;

            // Object Specific
            if (n.has_child("material")) n["material"] >> v->material;

            // Volume Specific
            if (n.has_child("skip_bytes")) n["skip_bytes"] >> v->skip_bytes;
            if (n.has_child("dimensions")) n["dimensions"] >> v->dimensions;
            if (n.has_child("voxel_type")) n["voxel_type"] >> v->voxel_type;

            // Camera Specific
            if (n.has_child("target")) n["target"] >> v->target;

            return true;
        }

        inline bool read(c4::yml::ConstNodeRef const& n, SceneElementConfig* v)
        {
            if (!n.is_map()) return false;

            if (n.has_child("name"))        n["name"] >> v->name;
            if (n.has_child("type"))        n["type"] >> v->type;
            if (n.has_child("pipelines"))   n["pipelines"] >> v->pipelines;
            if (n.has_child("data"))        n["data"] >> v->data;

            return true;
        }

        inline bool read(c4::yml::ConstNodeRef const& n, LightConfig::Data* v)
        {
            if (!n.is_map()) return false;

            if (n.has_child("color"))           n["color"] >> v->color;
            if (n.has_child("direction"))       n["direction"] >> v->direction;
            if (n.has_child("position"))        n["position"] >> v->position;
            if (n.has_child("intensity"))       n["intensity"] >> v->intensity;
            if (n.has_child("radius"))          n["radius"] >> v->radius;
            if (n.has_child("inner_cutoff"))    n["inner_cutoff"] >> v->inner_cutoff;
            if (n.has_child("outer_cutoff"))    n["outer_cutoff"] >> v->outer_cutoff;
            if (n.has_child("falloff"))         n["falloff"] >> v->falloff;

            return true;
        }

        inline bool read(c4::yml::ConstNodeRef const& n, LightConfig* v)
        {
            if (!n.is_map()) return false;

            if (n.has_child("name")) n["name"] >> v->name;
            if (n.has_child("type")) n["type"] >> v->type;
            if (n.has_child("pipelines")) n["pipelines"] >> v->pipelines;
            if (n.has_child("data")) n["data"] >> v->data;

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