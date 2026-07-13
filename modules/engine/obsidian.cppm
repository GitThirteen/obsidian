module;
#include <native/macros.h>

// =========================== //
//   ENGINE - OBSIDIAN MODULE  //
// =========================== //

export module Obsidian.Engine:Obsidian;
import Obsidian.Core;
//import Obsidian.Window;
import Obsidian.Graphics;

import :ResourceManager;

EXPORT(obsidian)

struct Obsidian
{
    //Window window;
    ResourceManager resources;
    //ShaderManager shaders;
    
    void run()
    {
        // Main loop logic goes here
    }
};

EXPORT_END