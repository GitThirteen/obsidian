#pragma once

#include <native/core/include.h>
#include <native/platform/window.obn.h>
#include <native/scene/camera.h>
#include <native/scene/light.h>
#include <native/scene/object.h>
#include <native/graphics/envmap.h>

class Scene;

using SceneInitCallback = std::function<void(Scene&, Root&, WindowManager&)>;
using SceneUpdateCallback = std::function<void(Scene&, double)>;

enum class SceneElement
{
	Unknown,
	Terrain,
	Object,
	Volume,
	Camera
};

class Scene
{
public:
	Scene(const std::string& name, SceneInitCallback on_init, SceneUpdateCallback on_update = nullptr) : m_name(name), m_on_init(on_init), m_on_update(on_update) { }

	auto initialize(Root& root, WindowManager& window) -> void;
	auto update(double dt) -> void;
	auto add_object(std::shared_ptr<SceneObject> obj) -> void;
	auto objects() const -> const std::vector<std::shared_ptr<SceneObject>>&;
	auto find_object(const std::string& name) -> SceneObject*;
	auto add_camera(std::shared_ptr<Camera> cam) -> void;
	auto active_camera() -> Camera&;
	auto cycle_camera() -> void;
	auto add_ambient_light(const AmbientLight& light) -> void;
	auto add_directional_light(const DirectionalLight& light) -> void;
	auto add_scene_light(const SceneLight& light) -> void;
	auto ambient_light() const -> const AmbientLight;
	auto directional_light() const -> const DirectionalLight;
	auto add_envmap(const std::string& path) -> void;
	auto envmap() const -> const EnvironmentMap&;
	auto envmap() -> EnvironmentMap&;
	auto lights() const -> const std::vector<SceneLight>&;
	auto name() const -> const std::string&;
	auto triangle_soup() const -> std::vector<TriangleMesh>;

private:
	std::string m_name = "";
	bool m_initialized = false;
	SceneInitCallback m_on_init = nullptr;
	SceneUpdateCallback m_on_update = nullptr;

	std::vector<std::shared_ptr<SceneObject>> m_objects;
	std::vector<std::shared_ptr<Camera>> m_cameras;
	size_t m_curr_camera_index = 0;

	AmbientLight m_ambient;
	DirectionalLight m_directional;
	std::vector<SceneLight> m_lights;
	EnvironmentMap m_envmap;
};

class SceneManager
{
public:
	SceneManager(Root& root, WindowManager& window, AssetManager& assets) : m_root(root), m_window(window), m_assets(assets) { }

	auto load(const std::string& scene_folder_path) -> void;
	auto switch_to(const std::string& name) -> void;
	auto active_scene() -> Scene&;
	auto update(double dt) -> void;
	auto process_input() -> void; // TODO: Should probably also go into the eventmanager!

private:
	Root& m_root;
	WindowManager& m_window;
	AssetManager& m_assets;

	std::unordered_map<std::string, std::shared_ptr<Scene>> m_scenes;
	std::shared_ptr<Scene> m_active_scene;

	auto create_scene(const std::string& name, SceneInitCallback init, SceneUpdateCallback update = nullptr) -> void;
};