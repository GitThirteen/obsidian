#pragma once

#include <native/_include.h>
#include <native/_define.h>
#include <native/root.h>
#include <native/input.h>
#include <native/scene.h>
#include <native/ui.h>

class Obsidian
{
public:
    Obsidian(Obsidian const&) = delete;
    void operator=(Obsidian const&) = delete;

    Root root;
    InputManager input;
    SceneManager scene;
    UIManager ui;

    void draw();
};