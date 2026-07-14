module;
#include <vulkan/vulkan_raii.hpp>
#include <native/macros.h>

// ================================ //
// ENGINE - RESOURCE MANAGER MODULE //
// ================================ //

export module Obsidian.Engine.ResourceManager;

import std;
import Obsidian.Core.Manager;
import Obsidian.Core.Result;
import Obsidian.Core.Logger;
import Obsidian.Core.Types;

EXPORT(obsidian)

class ResourceManager : public Manager
{
public:
	explicit ResourceManager(const vk::raii::Device& device) : m_device(&device) { }

    auto initialize() -> Result<void> override
    {
        log::info("Creating synchronization...");
        
        try
        {
            m_image_available_semaphores.reserve(FRAMES_IN_FLIGHT);
            m_in_flight_fences.reserve(FRAMES_IN_FLIGHT);

            for (int i = 0; i < FRAMES_IN_FLIGHT; ++i)
            {
                m_image_available_semaphores.emplace_back(*m_device, vk::SemaphoreCreateInfo{});
                m_in_flight_fences.emplace_back(
                    *m_device, vk::FenceCreateInfo{vk::FenceCreateFlagBits::eSignaled}
                );
            }
            
            return std::expected<void, std::string>();
        } 
        catch (const std::exception& e)
        {
            return std::unexpected(std::format("Failed to create synchronization: {}", e.what()));
        }
    }
    
    ~ResourceManager() = default;

private:
    const vk::raii::Device* m_device = nullptr;
    vk::raii::SwapchainKHR m_swapchain = nullptr;
    std::vector<vk::raii::Semaphore> m_image_available_semaphores;
    std::vector<vk::raii::Fence> m_in_flight_fences;
};

EXPORT_END