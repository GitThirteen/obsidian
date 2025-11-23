#include <native/core/obsidian/obsidian.h>

int main(int c, char* v[])
{
	Logger::initialize(c, v, "obsidian.log");

	Obsidian obsidian;

	obsidian.window.create();

	obsidian.shaders.load("./resources/shaders/active/", "./resources/shaders/compiled/"); // TODO: figure out how to compile GLSL -> SPIR-V in-cache instead of having to write the compiled version on disk during runtime, then read it
	obsidian.shaders.build_pipeline("test");

	obsidian.flow();
}