#pragma once

#include <native/core/obsidian/include.h>

#include <native/managers/scene.h>
#include <native/managers/window.h>
#include <native/managers/shaders.h>
#include <native/managers/event.h>
#include <native/managers/frame.h>

#include <native/util/timer.h>
#include <native/util/config.h>
#include <native/util/primitives.h>

class Obsidian
{
public:
    Root root;

    WindowManager window;
    SceneManager scenes;
    ShaderManager shaders;
    EventManager events;
    FrameManager<2> frame;

    Obsidian() : 
        window(root),
        scenes(root, window),
        shaders(root),
        events(window),
        frame(root, window)
    { }

    Obsidian(Obsidian const&) = delete;
    void operator=(Obsidian const&) = delete;

    /**
     * @brief Starts and runs the main render loop.
     */
    void flow();

private:
    FrameTimer m_timer;
};