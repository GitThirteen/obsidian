#include <native/scene/scene.obn.h>

void Scene::initialize(Root& root, WindowManager& window)
{
	if (this->m_initialized) return;

	if (this->m_on_init) {
		this->m_on_init(*this, root, window);
	}

	this->m_initialized = true;
}

void Scene::update(double dt)
{
	active_camera().update(dt);

	if (this->m_on_update) {
		this->m_on_update(*this, dt);
	}
}

void Scene::add_object(std::shared_ptr<SceneObject> obj)
{
	this->m_objects.push_back(std::move(obj));
}

const std::vector<std::shared_ptr<SceneObject>>& Scene::objects() const
{
	return this->m_objects;
}

SceneObject* Scene::find_object(const std::string& name)
{
	for (auto& obj : this->m_objects)
	{
		if (obj->name() == name)
		{
			return obj.get();
		}
	}

	LOG_S(WARNING) << "Object with name " << name << " could not be found!";

	return this->m_objects.empty() ? nullptr : this->m_objects[0].get();
}

void Scene::add_camera(std::shared_ptr<Camera> cam)
{
	this->m_cameras.push_back(std::move(cam));
}

Camera& Scene::active_camera()
{
	if (this->m_cameras.empty())
	{
		throw std::runtime_error("Scene " + this->m_name + " has no cameras!");
	}

	return *this->m_cameras[this->m_curr_camera_index];
}

void Scene::cycle_camera()
{
	if (this->m_cameras.empty()) return;

	this->m_curr_camera_index = (this->m_curr_camera_index + 1) % this->m_cameras.size();
	LOG_S(INFO) << "Switched to cam " << this->m_curr_camera_index;
}

const std::string& Scene::name() const
{
	return this->m_name;
}

void SceneManager::create_scene(const std::string& name, SceneInitCallback init, SceneUpdateCallback update)
{
	auto scene = std::make_shared<Scene>(name, init, update);
	this->m_scenes[name] = scene;
}

void SceneManager::switch_to(const std::string& name)
{
	auto it = this->m_scenes.find(name);
	if (it == this->m_scenes.end()) {
		LOG_S(ERROR) << "Scene " << name << " not found.";
		return;
	}

	this->m_active_scene = it->second;

	// This init is lazy - do we want this?
	m_active_scene->initialize(m_root, m_window);

	LOG_S(INFO) << "Switched to scene " << name;
}

Scene& SceneManager::active_scene()
{
	if (!m_active_scene)
	{
		throw std::runtime_error("No active scene!");
	}

	return *m_active_scene;
}

void SceneManager::update(double dt)
{
	if (!m_active_scene) return;
	
	m_active_scene->update(dt);
}

void SceneManager::process_input()
{
	if (!m_active_scene) return;

	static bool c_key_was_pressed = false;
	if (glfwGetKey(m_window.glfw_window(), GLFW_KEY_C) == GLFW_PRESS)
	{
		if (!c_key_was_pressed)
		{
			m_active_scene->cycle_camera();
			c_key_was_pressed = true;
		}
	}
	else
	{
		c_key_was_pressed = false;
	}
}
