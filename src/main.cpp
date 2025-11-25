#include <native/core/obsidian.h>
#include <native/scene/primitives.h>

int main(int c, char* v[])
{
	Logger::initialize(c, v, "obsidian.log");

	Obsidian obsidian;

	LOG_S(INFO) << "Initializing window.";
	obsidian.window.create();

	LOG_S(INFO) << "Initializing resources.";
	obsidian.resources.initialize();
	obsidian.frame.initialize();

	LOG_S(INFO) << "Initializing shaders.";
	obsidian.shaders.load("./resources/shaders/active/", "./resources/shaders/compiled/"); // TODO: figure out how to compile GLSL -> SPIR-V in-cache instead of having to write the compiled version on disk during runtime, then read it
	obsidian.shaders.build_pipeline("test", obsidian.resources.renderpass());

	// TODO: load this automatically from scene description
	LOG_S(INFO) << "Initializing scenes.";
	obsidian.scenes.create_scene("Testing",
		[](Scene& s, Root& root, WindowManager& window) {
			auto floor = Primitives::generate(root, GeometricPrimitiveType::Plane);
			floor->set_position({ 0.0f, -1.0f, 0.0f });
			floor->scale({ 10.0f, 1.0f, 10.0f });
			s.add_object(floor);

			auto cube = Primitives::generate(root, GeometricPrimitiveType::Cube);
			cube->set_position({ -1.5f, 0.0f, 0.0f });
			s.add_object(cube);

			auto sphere = Primitives::generate(root, GeometricPrimitiveType::Sphere);
			sphere->set_position({ 1.5f, 0.0f, 0.0f });
			s.add_object(sphere);

			auto camera = std::make_shared<UserCamera>(glm::vec3(0.0f, 1.0f, 5.0f), window.glfw_window());
			s.add_camera(camera);

			auto camera2 = std::make_shared<StaticCamera>(glm::vec3(0.0f, 1.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f));
			s.add_camera(camera2);
		},
		[](Scene& s, double dt) {
			if (auto* cube = s.find_object("Cube1")) {
				cube->rotate(30.0f * (float)dt, { 0, 1, 0 });
			}
			if (auto* sphere = s.find_object("Sphere1")) {
				static float time = 0.0f;
				time += (float)dt;
				sphere->set_position({ 1.5f, sin(time) * 0.5f, 0.0f });
			}
		}
	);

	obsidian.scenes.switch_to("Testing");

	LOG_S(INFO) << "Starting render loop.";
	obsidian.flow();

	LOG_S(INFO) << "Destroying renderer.";
	obsidian.shatter();
}