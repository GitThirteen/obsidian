module;
#include <vulkan/vulkan_raii.hpp>
#include <native/macros.h>

// =========================== //
//   ENGINE - OBSIDIAN MODULE  //
// =========================== //

export module Obsidian.Engine.Obsidian;

import Obsidian.Platform.Window;
import Obsidian.Graphics;
import Obsidian.Engine.ResourceManager;
import Obsidian.Graphics.Mantle;

EXPORT(obsidian)

struct Obsidian
{
    FrameManager frames;
    ResourceManager resources;
    WindowManager window;
    ShaderManager shaders;

    explicit Obsidian(const Mantle& mantle) :
        frames(*mantle.device, *mantle.command_pool),
        resources(*mantle.device),
		window(*mantle.instance),
        shaders()
    { }

    auto initialize() -> Result<void>
    {
		frames.initialize().expect("Successfully initialized frame handler.");
        resources.initialize().expect("Successfully initialized Vulkan resources.");
        window.initialize().expect("Successfully initialized window.");
        shaders.initialize().expect("Successfully initialized shader handle.");

        return { };
    }
    
    void flow()
    {
        
    }
};

EXPORT_END