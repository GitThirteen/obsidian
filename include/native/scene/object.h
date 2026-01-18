#pragma once

#include <native/core/include.h>
#include <native/core/root.h>
#include <native/graphics/descriptors.h>
#include <native/graphics/shaders.obn.h>
#include <native/scene/assets.obn.h>

struct SceneObject
{
	SceneObject(
		std::shared_ptr<MeshAsset> mesh,
		std::shared_ptr<MaterialAsset> material,
		const std::vector<std::string>& pipelines,
		std::string name = "Object"
	) : m_mesh(std::move(mesh)), m_material(std::move(material)), m_name(std::move(name)), m_pipelines(pipelines) { };

	auto create_descriptor_set(Root& root, avk::descriptor_pool& pool, const Pipeline& pipeline) -> void;

	auto set_position(const glm::vec3& pos) -> void;
	auto translate(const glm::vec3& offset) -> void;
	auto set_rotation(const glm::vec3& euler_angles) -> void;
	auto rotate(const glm::vec3& delta_euler_angles) -> void;
	auto set_scale(const glm::vec3& factor) -> void;
    auto scale(const glm::vec3& factor) -> void;

	auto name() const -> const std::string& { return this->m_name; }
	auto index_count() const -> const size_t { return m_mesh.get()->index_count; }
	auto index_buffer() const -> const avk::buffer& { return m_mesh.get()->index_buffer; }
	auto vertex_buffer() const -> const avk::buffer& { return m_mesh.get()->vertex_buffer; }
	auto vertices_list() const -> const std::vector<Vertex>& { return m_mesh.get()->data.vertices; }
	auto indices_list() const -> const std::vector<uint32_t>& { return m_mesh.get()->data.indices; }
	auto has_descriptor_set() const -> bool { return m_descriptor_set != nullptr; }
	auto descriptor_set() const -> const vk::DescriptorSet& { return m_descriptor_set; }
	auto model_matrix() const -> const glm::mat4 { return m_model_matrix; }
    auto material() const -> const MaterialAsset& { return *m_material; }
	auto pipelines() const -> const std::vector<std::string>& { return m_pipelines; }

private:
	std::string m_name;

	std::shared_ptr<MeshAsset> m_mesh;
	std::shared_ptr<MaterialAsset> m_material;

	vk::DescriptorSet m_descriptor_set = nullptr;
	std::vector<std::string> m_pipelines;

    glm::vec3 m_position{ 0.0f, 0.0f, 0.0f };
    glm::vec3 m_rotation{ 0.0f, 0.0f, 0.0f };
    glm::vec3 m_scale{ 1.0f, 1.0f, 1.0f };

    glm::mat4 m_model_matrix{ 1.0f };

	auto update_model_matrix() -> void;
};