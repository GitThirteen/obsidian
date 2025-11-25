#pragma once

#include <native/core/include.h>

#include <native/graphics/resources.obn.h>
#include <native/graphics/shaders.obn.h>
#include <native/graphics/frame.obn.h>
#include <native/platform/window.obn.h>
#include <native/platform/input.obn.h>
#include <native/scene/scene.obn.h>

#include <native/core/timer.h>
#include <native/core/config.h>

class Obsidian
{
public:
    Root root;

    WindowManager window;
    SceneManager scenes;
    ResourceManager resources;
    ShaderManager shaders;
    EventManager events;
    FrameManager<2> frame;

    Obsidian() : 
        window(root),
        scenes(root, window),
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
};