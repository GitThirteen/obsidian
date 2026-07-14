module;
#include <vulkan/vulkan_raii.hpp>
#include <native/macros.h>

// =========================== //
//   ENGINE - OBSIDIAN MODULE  //
// =========================== //

export module Obsidian.Engine.Obsidian;

//import Obsidian.Window;
import Obsidian.Graphics;
import Obsidian.Engine.ResourceManager;

EXPORT(obsidian)

struct Obsidian
{
    //Window window;
    FrameManager frames;
    ResourceManager resources;
    ShaderManager shaders;

    Obsidian(const vk::raii::Device& device, const vk::raii::CommandPool& pool) :
        frames(device, pool),
        resources(device),
        shaders()
    { }

    auto initialize() -> Result<void>
    {
		frames.initialize().expect("Successfully initialized frame handler.");
        resources.initialize().expect("Successfully initialized Vulkan resources.");
        shaders.initialize().expect("Successfully initialized shader handler.");

        return { };
    }
    
    void flow()
    {
        
    }
};

EXPORT_END