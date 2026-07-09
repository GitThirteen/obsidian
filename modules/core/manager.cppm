export module Obsidian.Core:Manager;
import std;
import :Result;

export namespace obsidian 
{
    class Manager 
    {
    public:
        virtual ~Manager() = default;
        virtual auto initialize() -> Result<void> = 0;
    };
}