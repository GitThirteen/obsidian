#include <native/core/obsidian.h>

/* !==== Neverending List of ToDo's ====! */
// [ ] Implement lighting (Ambient, Directional, Point, for now..)
// ===============================================================
// [ ] 2x/4x MSAA / TAA & more...
// [ ] Avoid write when compiling from GLSL to SPIR-V
// [ ] Add audio support
// [ ] Add a simple GUI
// [ ] LoD data structure
// [ ] Ray tracing intersection data structure (Octree, maybe later BVH?)
// [ ] Fix swapchain recreation on window resize
// [ ] Fix FPS capping accuracy
// [ ] Swap all AVK calls with regular VK calls (AVK too inconsistent)
// [ ] Refactor like, idk, half the renderer
// [ ] Physics? (Colliders, Actors, Gravity, ...)
// [ ] Raytracing!! Path Tracing? (Path Guiding...?)
/* !====================================! */

int main(int c, char* v[])
{
	try
	{
		Logger::initialize(c, v, "obsidian.log");

		Obsidian obsidian;

		LOG_S(INFO) << "Initializing window.";
		obsidian.window.create();

		LOG_S(INFO) << "Initializing resources.";
		obsidian.resources.initialize();
		obsidian.frame.initialize();

		LOG_S(INFO) << "Initializing shaders.";
		obsidian.shaders.load("./resources/shaders/active/", "./resources/shaders/compiled/");

		LOG_S(INFO) << "Initializing scenes.";
		obsidian.scenes.load("./resources/scenes/");

		LOG_S(INFO) << "Starting render loop.";
		obsidian.flow();

		LOG_S(INFO) << "Destroying renderer.";
		obsidian.shatter();

		return EXIT_SUCCESS;
	}
	catch (const std::exception& e)
	{
		LOG_S(ERROR) << "Unhandled exception: " << e.what();
		return EXIT_FAILURE;
	}
}