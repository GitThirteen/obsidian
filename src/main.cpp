import std;
import Obsidian.Core;
import Obsidian.Engine;
import Obsidian.Graphics;
import Obsidian.Magma;

using namespace obsidian;

auto main(int argc, char* argv[]) -> int
{
    log::info("Launching Obsidian Engine...");

    Obsidian engine;
    
    log::info("Starting render loop.");
    engine.run();

    auto sampler = magma::sampler::Independent();
    auto sample = sampler.next_1d();
    auto sample_2d = sampler.next_2d();

    log::info("Samples: [1d] {}, [2d] {} {}", sample, sample_2d.x(), sample_2d.y());

    log::info("Shutting down engine.");
    return 0;
}