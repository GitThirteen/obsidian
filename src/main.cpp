import std;
import Obsidian.Core;
import Obsidian.Engine;

using namespace obsidian;

auto main(int argc, char* argv[]) -> int
{
    log::info("Launching Obsidian Engine...");

    Obsidian engine;
    
    log::info("Starting render loop.");
    engine.run();

    log::info("Shutting down engine.");
    return 0;
}