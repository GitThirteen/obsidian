module;
#include <vulkan/vulkan_raii.hpp>
#include <native/macros.h>

// =============================== //
// GRAPHICS - FRAME MANAGER MODULE //
// =============================== //

export module Obsidian.Graphics.FrameManager;

import std;
import Obsidian.Core.Manager;
import Obsidian.Core.Result;
import Obsidian.Core.Types;
import Obsidian.Graphics.Pipeline;
import Obsidian.Graphics.Shard;

EXPORT(obsidian)

class FrameManager : public Manager
{
public:
    FrameManager(const vk::raii::Device& device, const vk::raii::CommandPool& pool) : m_device(&device), m_pool(&pool) { }

    auto initialize() -> Result<void> override
    {
        if (!m_device || !m_pool)
        {
			return std::unexpected<std::string>("FrameManager requires a valid device and command pool.");
        }

        try
        {
            vk::CommandBufferAllocateInfo alloc_info;
			alloc_info.setCommandPool(**m_pool);
            alloc_info.setLevel(vk::CommandBufferLevel::ePrimary);
            alloc_info.setCommandBufferCount(FRAMES_IN_FLIGHT);

            m_frames = vk::raii::CommandBuffers(*m_device, alloc_info);
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
        m_curr_frame_index = (m_curr_frame_index + 1) % FRAMES_IN_FLIGHT;
    }
    
    auto set_active_image(const uint32_t index) -> void
    {
        m_curr_image_index = index;
    }

private:
    const vk::raii::Device* m_device = nullptr;
    const vk::raii::CommandPool* m_pool = nullptr;

    std::vector<vk::raii::CommandBuffer> m_frames;
    uint32_t m_curr_frame_index = 0;
    uint32_t m_curr_image_index = 0;
};

EXPORT_END