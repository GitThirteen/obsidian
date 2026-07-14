module;
#include <native/macros.h>

// =========================== //
//    CORE - MANAGER MODULE    //
// =========================== //

export module Obsidian.Core.Manager;
import std;
import Obsidian.Core.Result;

EXPORT(obsidian)

class Manager 
{
public:
    virtual ~Manager() = default;
    virtual auto initialize() -> Result<void> = 0;
};

EXPORT_END