#pragma once

#include <native/core/include.h>

#include <native/graphics/resources.obn.h>
#include <native/graphics/shaders.obn.h>
#include <native/graphics/frame.obn.h>
#include <native/platform/window.obn.h>
#include <native/platform/input.obn.h>
#include <native/scene/assets.obn.h>
#include <native/scene/scene.obn.h>

#include <native/core/timer.h>
#include <native/core/config.h>
#include <native/core/image_barrier.h>

class Obsidian
{
public:
    Root root;

    WindowManager window;
    AssetManager assets;
    SceneManager scenes;
    ResourceManager resources;
    ShaderManager shaders;
    EventManager events;
    FrameManager<2> frame;

    Obsidian() : 
        window(root),
        assets(root),
        scenes(root, window, assets),
        resources(root, window),
        shaders(root),
        events(window),
        frame(resources)
    { }

    Obsidian(Obsidian const&) = delete;
    void operator=(Obsidian const&) = delete;

    /**
     * @brief Starts and runs the main render loop.
     */
    auto flow() -> void;
    auto shatter() -> void;

private:
    FrameTimer m_timer;
    
    std::vector<avk::buffer> m_camera_buffers;
    std::vector<avk::buffer> m_light_buffers;
    std::vector<vk::DescriptorSet> m_global_descriptor_sets;

    avk::sampler m_envmap_sampler;

    auto command_list(Scene& scene, const std::vector<std::string>& shard_order, vk::DescriptorSet global_set) -> std::vector<avk::recorded_commands_t>;
};