module;
#include <vulkan/vulkan_raii.hpp>
#include <native/macros.h>

// =============================== //
// GRAPHICS - FRAME MANAGER MODULE //
// =============================== //

export module Obsidian.Graphics:FrameManager;
import std;
import Obsidian.Core;
import :Pipeline;
import :Shard;

EXPORT(obsidian)

class FrameManager 
{
public:
    FrameManager() = default;

    auto initialize(const vk::raii::Device& device, const vk::raii::CommandPool& pool, uint32_t max_frames_in_flight) -> Result<void> 
    {
        m_max_frames = max_frames_in_flight;
        try
        {
            vk::CommandBufferAllocateInfo alloc_info;
            alloc_info.setCommandPool(*pool);
            alloc_info.setLevel(vk::CommandBufferLevel::ePrimary);
            alloc_info.setCommandBufferCount(m_max_frames);

            m_frames = vk::raii::CommandBuffers(device, alloc_info);
            return {};
        } 
        catch (const std::exception& e)
        {
            return std::unexpected(std::format("Failed to allocate frame command buffers: {}", e.what()));
        }
    }

    auto current_command_buffer() -> const vk::raii::CommandBuffer& { return m_frames[m_curr_frame_index]; }
    auto current_image_index() const -> uint32_t { return m_curr_image_index; }
    auto current_frame_index() const -> uint32_t { return m_curr_frame_index; }

    auto advance_frame() -> void
    {
        m_curr_frame_index = (m_curr_frame_index + 1) % m_max_frames;
    }
    
    auto set_active_image(uint32_t index) -> void
    {
        m_curr_image_index = index;
    }

private:
    std::vector<vk::raii::CommandBuffer> m_frames;
    uint32_t m_curr_frame_index = 0;
    uint32_t m_curr_image_index = 0;
    uint32_t m_max_frames = 2;
};

EXPORT_END