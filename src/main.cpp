#include <native/core/obsidian.h>

int main(int c, char* v[])
{
	Logger::initialize(c, v, "obsidian.log");

	Obsidian obsidian;

	obsidian.window.create();

	obsidian.shaders.load("./shaders/active/");
	obsidian.shaders.build_pipeline("triangle");
}