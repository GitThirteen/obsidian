module;
#include <vulkan/vulkan_raii.hpp>
#include <native/macros.h>

// ============================ //
//   GRAPHICS - MANTLE MODULE   //
// ============================ //

export module Obsidian.Graphics.Mantle;

import std;
import Obsidian.Core.Result;
import Obsidian.Core.Logger;
import Obsidian.Core.Config;

EXPORT(obsidian)

struct Mantle
{
    std::unique_ptr<vk::raii::Context> context;
    std::unique_ptr<vk::raii::Instance> instance;
    std::unique_ptr<vk::raii::DebugUtilsMessengerEXT> debug_messenger;
    std::unique_ptr<vk::raii::PhysicalDevice> physical_device;
    std::unique_ptr<vk::raii::Device> device;
    std::unique_ptr<vk::raii::Queue> graphics_queue;
    std::unique_ptr<vk::raii::CommandPool> command_pool;

    std::uint32_t graphics_queue_family = 0;

    auto initialize() -> Result<void>
    {
        log::info("Establishing Vulkan bridge...");

        try
        {
            return launch();
        }
        catch (const std::exception& e)
        {
            return std::unexpected(std::format("Vulkan Initialization Failed: {}", e.what()));
        }
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT type,
        const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
        void* p_user_data)
    {
        if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            log::warn("Vulkan Validation: {}", p_callback_data->pMessage);
        }
        return VK_FALSE;
    }

private:
    auto launch() -> Result<void>
    {
        context = std::make_unique<vk::raii::Context>();

        std::string app_name = Config::get<std::string>("title").unwrap_or("Obsidian Engine");
        vk::ApplicationInfo app_info(app_name.c_str(), 1, "Obsidian", 1, VK_API_VERSION_1_4);

        std::vector<const char*> layers = { "VK_LAYER_KHRONOS_validation" };
        std::vector<const char*> extensions = {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME
        };

#ifdef _WIN32
        extensions.push_back("VK_KHR_win32_surface");
#endif

        vk::InstanceCreateInfo instance_info({}, &app_info, layers, extensions);
        instance = std::make_unique<vk::raii::Instance>(*context, instance_info);

        vk::DebugUtilsMessengerCreateInfoEXT debug_info(
            {},
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
            debug_callback
        );
        debug_messenger = std::make_unique<vk::raii::DebugUtilsMessengerEXT>(*instance, debug_info);

        vk::raii::PhysicalDevices physical_devices(*instance);
        physical_device = std::make_unique<vk::raii::PhysicalDevice>(std::move(physical_devices.front()));

        auto queue_families = physical_device->getQueueFamilyProperties();
        for (uint32_t i = 0; i < queue_families.size(); i++)
        {
            if (queue_families[i].queueFlags & vk::QueueFlagBits::eGraphics)
            {
                graphics_queue_family = i;
                break;
            }
        }

        vk::PhysicalDeviceDescriptorIndexingFeatures descriptor_indexing_features{};
        descriptor_indexing_features.setRuntimeDescriptorArray(VK_TRUE);
        descriptor_indexing_features.setDescriptorBindingPartiallyBound(VK_TRUE);
        descriptor_indexing_features.setDescriptorBindingVariableDescriptorCount(VK_TRUE);

        vk::PhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features{};
        dynamic_rendering_features.setDynamicRendering(VK_TRUE);
        dynamic_rendering_features.setPNext(&descriptor_indexing_features);

        vk::PhysicalDeviceScalarBlockLayoutFeatures scalar_features{};
        scalar_features.setScalarBlockLayout(VK_TRUE);
        scalar_features.setPNext(&dynamic_rendering_features);

        std::vector<const char*> device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        float queue_priority = 1.0f;
        vk::DeviceQueueCreateInfo queue_info({}, graphics_queue_family, 1, &queue_priority);

        vk::DeviceCreateInfo device_info({}, queue_info, {}, device_extensions, nullptr, &scalar_features);
        device = std::make_unique<vk::raii::Device>(*physical_device, device_info);

        graphics_queue = std::make_unique<vk::raii::Queue>(*device, graphics_queue_family, 0);

        vk::CommandPoolCreateInfo pool_info(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, graphics_queue_family);
        command_pool = std::make_unique<vk::raii::CommandPool>(*device, pool_info);

        return {};
    }
};

EXPORT_END