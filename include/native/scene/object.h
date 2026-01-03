#pragma once

#include <native/core/include.h>
#include <native/core/root.h>
#include <native/graphics/descriptors.h>
#include <native/graphics/shaders.obn.h>

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 uv;
};

struct GeometryData
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};

struct Material
{
	std::string name = "default";
	glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
	std::string pipeline = "";
};

struct SceneObject
{
	SceneObject(Root& root, GeometryData data, std::string name = "Object");

	auto create_descriptor_set(avk::descriptor_pool& pool, const Pipeline& pipeline, Root& root) -> void;

	auto set_position(const glm::vec3& pos) -> void;
	auto translate(const glm::vec3& offset) -> void;
	auto scale(const glm::vec3& factor) -> void;
	auto rotate(float angleDeg, const glm::vec3& axis) -> void;

	// TODO: Mirroring & Shearing? Local vs. global trafos?

	auto name() const -> const std::string& { return this->m_name; }
	auto index_count() const -> const size_t { return m_geometry.indices.size(); }
	auto has_descriptor_set() const -> bool { return m_descriptor_set != nullptr; }
	auto descriptor_set() const -> const vk::DescriptorSet& { return m_descriptor_set; }
	auto model_matrix() const -> const glm::mat4 { return m_model_matrix; }
	auto index_buffer() const -> const avk::buffer { return m_index_buffer; }
	auto vertex_buffer() const -> const avk::buffer { return m_vertex_buffer; }	

private:
	std::string m_name;
	GeometryData m_geometry;
	Material m_material;
	
	vk::DescriptorSet m_descriptor_set = nullptr;
	glm::mat4 m_model_matrix{ 1.0f };
	avk::buffer m_vertex_buffer;
	avk::buffer m_index_buffer;
};