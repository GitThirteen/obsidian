#pragma once

#include <native/core/include.h>
#include <native/core/root.h>
#include <native/scene/light.h>

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
};

struct GeometryData
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

struct CameraData
{
    glm::mat4 view_proj;
    glm::vec4 pos;
};

struct GPULightBlock
{
    AmbientLight ambient;
    DirectionalLight directional;
    SceneLight scene_lights[16];
};

enum class GeometricPrimitiveType
{
    Invalid,
    Plane,
    Cube,
    Sphere
};

struct VolumeData
{
    std::vector<uint8_t> data;
    glm::ivec3 dimensions;
    std::string format;

    auto bytes_per_voxel() const -> const int
    {
        if (format == "uint8") return 1;
        if (format == "uint12") return 2;
        if (format == "uint16") return 2;
        if (format == "float32") return 4;

        throw std::runtime_error("Unknown voxel type: " + format);
    }

    auto vk_format() const -> const vk::Format
    {
        if (format == "uint8") return vk::Format::eR8Unorm;
        if (format == "uint12") return vk::Format::eR16Unorm;
        if (format == "uint16") return vk::Format::eR16Unorm;
        if (format == "float32") return vk::Format::eR32Sfloat;

        throw std::runtime_error("Unknown voxel type: " + format);
    }
};

struct Primitives
{
    static auto generate(GeometricPrimitiveType type) -> GeometryData;

    static auto _generate_plane_gm() -> GeometryData;
    static auto _generate_cube_gm() -> GeometryData;
    static auto _generate_sphere_gm(int sectors, int stacks) -> GeometryData;
};

struct MeshAsset
{
    avk::buffer vertex_buffer;
    avk::buffer index_buffer;
    uint32_t index_count = 0;
};

struct TextureAsset
{
    avk::image image;
    avk::image_sampler handle;
    bool is_volume = false;
};

struct MaterialUniforms
{
    glm::vec4 base_color = { 1.0f, 1.0f, 1.0f, 1.0f };
    glm::vec4 params; // x = metallic; y = roughness; z, w = free
};

struct MaterialAsset
{
    std::string name;

    glm::vec4 base_color = { 1.0f, 1.0f, 1.0f, 1.0f };
    float metallic = 0.0f;
    float roughness = 1.0f;

    avk::buffer ubo;

    std::shared_ptr<TextureAsset> albedo_map = nullptr;
    std::shared_ptr<TextureAsset> normal_map = nullptr;
    std::shared_ptr<TextureAsset> roughness_map = nullptr;
    std::shared_ptr<TextureAsset> ao_map = nullptr;

    std::shared_ptr<TextureAsset> volume_map = nullptr;
    auto is_volume() const -> bool { return volume_map != nullptr; }
};

class AssetManager
{
public:
    AssetManager(Root& root);

    auto mesh(const std::string& name) -> std::shared_ptr<MeshAsset>;
    auto add_mesh(const std::string& name, const std::string& filepath) -> std::shared_ptr<MeshAsset>;
    auto has_mesh(const std::string& name) -> bool;

    auto texture(const std::string& name) -> std::shared_ptr<TextureAsset>;
    auto add_texture(const std::string& name, const std::string& filepath) -> std::shared_ptr<TextureAsset>;
    auto add_texture(const std::string& name, const VolumeData& data) -> std::shared_ptr<TextureAsset>;
    auto has_texture(const std::string& name) -> bool;

    auto material(const std::string& name) -> std::shared_ptr<MaterialAsset>;
    auto add_material(const std::string& name, const MaterialAsset& material) -> std::shared_ptr<MaterialAsset>;
    auto has_material(const std::string& name) -> bool;

    static auto load_obj(const std::string& filepath) -> GeometryData;
    static auto load_vol(const std::string& filepath, const std::array<int, 3>& dims, const std::string& voxel_type, size_t skip_bytes) -> VolumeData;

private:
    Root& m_root;

    std::unordered_map<std::string, std::shared_ptr<MeshAsset>> m_meshes; // TODO: store solid entities & use index-based referencing
    std::unordered_map<std::string, std::shared_ptr<TextureAsset>> m_textures;
    std::unordered_map<std::string, std::shared_ptr<MaterialAsset>> m_materials;

    auto generic_texture(const std::array<uint8_t, 4>& data) -> TextureAsset;
    auto upload_image_data(avk::buffer& staging, avk::image& target) -> void;
};

struct IndexHash {
    std::size_t operator()(const tinyobj::index_t& k) const {
        return ((std::hash<int>()(k.vertex_index) ^
            (std::hash<int>()(k.normal_index) << 1)) >> 1) ^
            (std::hash<int>()(k.texcoord_index) << 1);
    }
};

struct IndexEqual {
    bool operator()(const tinyobj::index_t& lhs, const tinyobj::index_t& rhs) const {
        return lhs.vertex_index == rhs.vertex_index &&
            lhs.normal_index == rhs.normal_index &&
            lhs.texcoord_index == rhs.texcoord_index;
    }
};