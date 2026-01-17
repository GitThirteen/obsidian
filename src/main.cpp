#include <native/core/obsidian.h>

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