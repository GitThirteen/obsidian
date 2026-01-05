#include <native/scene/scene.obn.h>

void Scene::initialize(Root& root, WindowManager& window)
{
	if (this->m_initialized) return;

	if (this->m_on_init)
	{
		this->m_on_init(*this, root, window);
	}

	this->m_initialized = true;
}

void Scene::update(double dt)
{
	active_camera().update(dt);

	if (this->m_on_update)
	{
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

auto SceneManager::load(const std::string& scenes_folder_path) -> void
{
	auto string_to_scene_obj = [](const std::string& type) {
		if (type == "terrain")  return SceneElement::Terrain;
		if (type == "object")   return SceneElement::Object;
		if (type == "volume")   return SceneElement::Volume;
		if (type == "light")    return SceneElement::Light;
		if (type == "static_camera" || type == "user_camera" || "dynamic_camera") return SceneElement::Camera;
		return SceneElement::Unknown;
	};

	std::string first_scene = "";
	const auto& scene_order = Config::get<std::vector<std::string>>("scenes");
	for (const auto& scene_name : scene_order)
	{
		if (m_scenes.find(scene_name) != m_scenes.end())
		{
			LOG_S(WARNING) << "There already is a scene with the name '" << scene_name << "'. Scene names must be unique. Skipping.";
			continue;
		}

		try
		{
			Config::read_to_local(scenes_folder_path + scene_name + ".yaml");
		}
		catch (const std::exception& e)
		{
			LOG_S(ERROR) << "Failed to read scene file for scene " << scene_name << ": " << e.what();
			continue;
        }

		Scene scene(scene_name, nullptr, nullptr);

        const auto& scene_elements = Config::get<std::vector<SceneElementConfig>>("elements");
		const auto& scene_materials = Config::get<std::vector<MaterialConfig>>("materials");

		for (const auto& material : scene_materials)
		{
			if (material.name.empty())
			{
				LOG_S(WARNING) << "Attempting to use a material in scene '" << scene_name << "' without a name. Skipping.";
				continue;
			}

			if (m_assets.has_material(material.name))
			{
				LOG_S(WARNING) << "Material '" << material.name << "' already exists in the assets. Skipping.";
				continue;
			}

			MaterialAsset asset;
			asset.name = material.name;
			asset.base_color = { material.base_color[0], material.base_color[1], material.base_color[2], material.base_color[3] };
			asset.metallic = material.metallic;
			asset.roughness = material.roughness;

			if (!material.textures.albedo.empty()) {
				asset.albedo_map = m_assets.add_texture(material.textures.albedo, material.textures.albedo);
			}

			if (!material.textures.normal.empty()) {
				asset.normal_map = m_assets.add_texture(material.textures.normal, material.textures.normal);
			}

			if (!material.textures.roughness.empty()) {
				asset.roughness_map = m_assets.add_texture(material.textures.roughness, material.textures.roughness);
			}

			if (!material.textures.ao.empty()) {
				asset.ao_map = m_assets.add_texture(material.textures.ao, material.textures.ao);
			}
			
			m_assets.add_material(asset.name, asset);
		}
		
		for (const auto& element : scene_elements)
		{
			if (element.name.empty())
			{
				LOG_S(WARNING) << "Attempting to use an element in scenee '" << scene_name << "' without a name. Skipping.";
				continue;
			}

			if (element.type.empty())
			{
				LOG_S(WARNING) << "Attempting to use element '" << element.name << "' in scene '" << scene_name << "' without a type. Skipping.";
				continue;
			}
			
			auto element_type = string_to_scene_obj(element.type);
			if (element_type == SceneElement::Unknown)
			{
				LOG_S(WARNING) << "Reading unknown element type during loading of scene '" << scene_name << "'. Did you make a typo somewhere?";
				continue;
			}

			if (element_type == SceneElement::Terrain || element_type == SceneElement::Object)
			{
				if (element.data.file.empty() || element.data.material.empty() || element.data.position.empty())
				{
					LOG_S(WARNING) << "Attempting to create terrain or object, but not all required fields have values. Skipping.";
					continue;
				}

				std::string mesh_name = std::filesystem::path(element.data.file).stem().string();
				auto mesh = m_assets.add_mesh(mesh_name, element.data.file);
				auto material = m_assets.material(element.data.material);
				SceneObject object(mesh, material, element.pipelines, element.name);
				
				const auto& pos = element.data.position;
				object.set_position({ pos[0], pos[1], pos[2] });
				const auto& rot = element.data.rotate;
				object.set_rotation({ rot[0], rot[1], rot[2] });
				const auto& scl = element.data.scale;
				object.set_scale({ scl[0], scl[1], scl[2] });

				auto obj_ptr = std::make_shared<SceneObject>(object);
				scene.add_object(std::move(obj_ptr));
				
				continue;
			}

			if (element_type == SceneElement::Volume)
			{
				auto mesh_ptr = m_assets.add_mesh(element.name, "OBSIDIAN_CUBE");

				VolumeData vd = m_assets.load_vol(element.data.file, element.data.dimensions, element.data.voxel_type, element.data.skip_bytes);
				auto vd_ptr = m_assets.add_texture(element.name, vd);
				
				MaterialAsset material;
				material.name = element.name;
				material.volume_map = vd_ptr;
				auto mat_ptr = m_assets.add_material(element.name, material);

				SceneObject object(mesh_ptr, mat_ptr, element.pipelines, element.name);
				const auto& pos = element.data.position;
				object.set_position({ pos[0], pos[1], pos[2] });
				const auto& rot = element.data.rotate;
				object.set_rotation({ rot[0], rot[1], rot[2] });
				const auto& scl = element.data.scale;
				object.set_scale({ scl[0], scl[1], scl[2] });

				auto obj_ptr = std::make_shared<SceneObject>(object);
				scene.add_object(std::move(obj_ptr));

				continue;
			}

			if (element_type == SceneElement::Light)
			{
				continue;
			}

			if (element_type == SceneElement::Camera)
			{
				if (element.type == "user_camera")
				{
					const auto& pos = element.data.position;
					UserCamera user_cam({ pos[0], pos[1], pos[2] }, m_window.glfw_window());

					scene.add_camera(
						std::make_shared<UserCamera>(std::move(user_cam))
					);

					continue;
				}
				
				if (element.type == "static_camera")
				{
					const auto& pos = element.data.position;
					const auto& target = element.data.target;
					StaticCamera static_cam({ pos[0], pos[1], pos[2] }, { target[0], target[1], target[2] });

					scene.add_camera(
						std::make_shared<StaticCamera>(std::move(static_cam))
					);

					continue;
				}

				if (element.type == "dynamic_camera")
				{
					const auto& pos = element.data.position;
					DynamicCamera dynamic_cam({ pos[0], pos[1], pos[2] });

					scene.add_camera(
						std::make_shared<DynamicCamera>(std::move(dynamic_cam))
					);

					continue;
				}
			}
		}

		scene.initialize(m_root, m_window);
		m_scenes.emplace(scene_name, std::make_shared<Scene>(std::move(scene)));
		
		if (first_scene.empty())
		{
			first_scene = scene_name;
		}
	}

	if (m_scenes.empty())
	{
		LOG_S(ERROR) << "No scenes available to render! Aborting.";
		exit(0);
	}

	Config::read_to_local(); // Make the config read the main yaml again
	switch_to(first_scene);
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
