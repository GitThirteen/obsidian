module;
#include <native/macros.h>

// =========================== //
//        ENGINE MODULE        //
// =========================== //

export module Obsidian.Engine;
import Obsidian.Core;
//import Obsidian.Window;
//import Obsidian.Graphics;

import :ResourceManager;

EXPORT_OBSIDIAN

struct Obsidian
{
    //Window window;
    //ResourceManager resources;
    //ShaderManager shaders;
    
    void run()
    {
        // Main loop logic goes here
    }
};

EXPORT_END