#pragma once

#include <native/core/include.h>
#include <native/platform/window.obn.h>
#include <native/scene/camera.h>
#include <native/scene/object.h>
#include <native/scene/light.h>

class Scene;

using SceneInitCallback = std::function<void(Scene&, Root&, WindowManager&)>;
using SceneUpdateCallback = std::function<void(Scene&, double)>;

class Scene
{
public:
	Scene(const std::string& name, SceneInitCallback on_init, SceneUpdateCallback on_update = nullptr) : m_name(name), m_on_init(on_init), m_on_update(on_update) { }

	void initialize(Root& root, WindowManager& window);
	void update(double dt);
	void add_object(std::shared_ptr<SceneObject> obj);
	const std::vector<std::shared_ptr<SceneObject>>& objects() const;
	SceneObject* find_object(const std::string& name);
	void add_camera(std::shared_ptr<Camera> cam);
	Camera& active_camera();
	void cycle_camera();
	const std::string& name() const;

private:
	std::string m_name = "";
	bool m_initialized = false;
	SceneInitCallback m_on_init = nullptr;
	SceneUpdateCallback m_on_update = nullptr;

	std::vector<std::shared_ptr<SceneObject>> m_objects;
	std::vector<std::shared_ptr<LightSource>> m_lights; // TODO: Lights are currently WIP, this is just a dummy for now.
	std::vector<std::shared_ptr<Camera>> m_cameras;
	size_t m_curr_camera_index = 0;
};

class SceneManager
{
public:
	SceneManager(Root& root, WindowManager& window) : m_root(root), m_window(window) { }

	void create_scene(const std::string& name, SceneInitCallback init, SceneUpdateCallback update = nullptr);
	void switch_to(const std::string& name);
	Scene& active_scene();
	void update(double dt);
	void process_input(); // TODO: Should probably also go into the eventmanager!

private:
	Root& m_root;
	WindowManager& m_window;

	std::unordered_map<std::string, std::shared_ptr<Scene>> m_scenes;
	std::shared_ptr<Scene> m_active_scene;
};