#pragma once

#include <native/core/include.h>

#include <native/managers/input.h>
#include <native/managers/scene.h>
#include <native/managers/window.h>
#include <native/managers/shaders.h>

class Obsidian
{
public:
    static Root root;

    InputManager input;
    SceneManager scene;
    WindowManager window;
    ShaderManager shaders;

    Obsidian() : 
        window(root),
        shaders(root)
    { }

    Obsidian(Obsidian const&) = delete;
    void operator=(Obsidian const&) = delete;

    //void draw();
};