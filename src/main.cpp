import std;
import Obsidian.Core;
import Obsidian.Engine;
import Obsidian.Graphics;
import Obsidian.Magma;
import Obsidian.Graphics.Mantle;

using namespace obsidian;

auto main(int argc, char* argv[]) -> int
{
    log::info("Preparing Obsidian Engine...");

    Mantle bridge;
    bridge.initialize().expect("Finished building Vulkan bridge.");

    Obsidian engine(bridge);
    engine.initialize().expect("Finished building engine components.");
    engine.flow();

    log::info("Shutting down engine.");
    return 0;
}