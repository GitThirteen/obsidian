module;
#include <vulkan/vulkan_raii.hpp>
export module Obsidian.Graphics:Pipeline;
import std;
import Obsidian.Core;

export namespace obsidian 
{
    struct PipelineOptions 
    {
        vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList;
        vk::PolygonMode polygon_mode = vk::PolygonMode::eFill;
        vk::CullModeFlags cull_mode = vk::CullModeFlagBits::eBack;
        vk::FrontFace front_face = vk::FrontFace::eCounterClockwise;
        vk::CompareOp compare_operation = vk::CompareOp::eGreaterOrEqual;
        bool depth_test = true;
        bool depth_write = true;
        std::string blending = "none";

        enum class Type { Default, Wireframe, Transparent, Additive };

        static auto create_default() -> PipelineOptions
        {
            return PipelineOptions{};
        }

        static auto create_wireframe() -> PipelineOptions
        {
            PipelineOptions options;
            options.polygon_mode = vk::PolygonMode::eLine;
            options.cull_mode = vk::CullModeFlagBits::eNone;
            return options;
        }

        static auto create_transparent(std::string blend = "alpha") -> PipelineOptions
        {
            PipelineOptions options;
            options.depth_test = false;
            options.depth_write = false;
            options.blending = blend;
            return options;
        }
    };

    struct PipelineKey 
    {
        std::string name;
        std::vector<vk::Format> colors;
        vk::Format depth;
        PipelineOptions::Type type;

        auto operator==(const PipelineKey& other) const -> bool {
            return name == other.name && colors == other.colors && depth == other.depth && type == other.type;
        }
    };

    struct PipelineKeyHash 
    {
        auto operator()(const PipelineKey& key) const -> std::size_t {
            std::size_t h = std::hash<std::string>{}(key.name);
            for (auto f : key.colors) h ^= std::hash<int>{}(static_cast<int>(f));
            h ^= std::hash<int>{}(static_cast<int>(key.depth));
            h ^= std::hash<int>{}(static_cast<int>(key.type));
            return h;
        }
    };

    class Pipeline 
    {
    private:
        vk::raii::Pipeline m_pipeline = nullptr;
        vk::raii::PipelineLayout m_layout = nullptr;
        std::vector<vk::raii::DescriptorSetLayout> m_descriptor_layouts;
        vk::PipelineBindPoint m_bind_point;

    public:
        Pipeline() = default;
        
        Pipeline(vk::raii::Pipeline pipeline, vk::raii::PipelineLayout layout, std::vector<vk::raii::DescriptorSetLayout> desc_layouts, vk::PipelineBindPoint bind_point)
            : m_pipeline(std::move(pipeline)), m_layout(std::move(layout)), m_descriptor_layouts(std::move(desc_layouts)), m_bind_point(bind_point) { }

        auto bind_into(const vk::raii::CommandBuffer& cmd_buffer) const -> void
        {
            cmd_buffer.bindPipeline(m_bind_point, *m_pipeline);
        }

        auto layout_handle() const -> vk::PipelineLayout
        {
            return *m_layout;
        }

        auto make_descriptor_set(const vk::raii::Device& device, const vk::raii::DescriptorPool& pool, uint32_t set_index = 0) const -> Result<vk::raii::DescriptorSet>
        {
            try
            {
                vk::DescriptorSetAllocateInfo alloc_info;
                alloc_info.setDescriptorPool(*pool);
                alloc_info.setSetLayouts(*m_descriptor_layouts[set_index]);
                
                vk::raii::DescriptorSets sets(device, alloc_info);
                return std::move(sets[0]);
            } 
            catch (const std::exception& e)
            {
                return std::unexpected(std::format("Failed to allocate descriptor set: {}", e.what()));
            }
        }
    };
}