module;
#include <vulkan/vulkan_raii.hpp>
#include <native/macros.h>

// ================================ //
// ENGINE - RESOURCE MANAGER MODULE //
// ================================ //

export module Obsidian.Engine:ResourceManager;
import std;
import Obsidian.Core;

EXPORT(obsidian)

class ResourceManager
{
public:
    auto initialize(const vk::raii::Device& device) -> Result<void>
    {
        log::info("Creating synchronization...");
        
        try
        {
            int frames = 2; // TODO: Fetch this from the config
            m_image_available_semaphores.reserve(frames);
            m_in_flight_fences.reserve(frames);

            for (int i = 0; i < frames; ++i)
            {
                m_image_available_semaphores.emplace_back(device, vk::SemaphoreCreateInfo{});
                m_in_flight_fences.emplace_back(
                    device, vk::FenceCreateInfo{vk::FenceCreateFlagBits::eSignaled}
                );
            }
            
            return std::expected<void, std::string>();
        } 
        catch (const std::exception& e)
        {
            return std::unexpected(std::format("Failed to create syncers: {}", e.what()));
        }
    }
    
    ~ResourceManager() = default;

private:
    vk::raii::SwapchainKHR m_swapchain = nullptr;
    std::vector<vk::raii::Semaphore> m_image_available_semaphores;
    std::vector<vk::raii::Fence> m_in_flight_fences;
};

EXPORT_END