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

    // TODO: The rest, lol
};

EXPORT_END