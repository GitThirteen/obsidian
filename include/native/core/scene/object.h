#pragma once

#include <native/core/obsidian/include.h>
#include <native/core/obsidian/root.h>

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
	// TODO: Hide those
	glm::mat4 m_model_matrix{ 1.0f };
	avk::buffer m_vertex_buffer;
	avk::buffer m_index_buffer;

	SceneObject(Root& root, GeometryData data, std::string name = "Object");

	void set_position(const glm::vec3& pos);
	void translate(const glm::vec3& offset);
	void scale(const glm::vec3& factor);
	void rotate(float angleDeg, const glm::vec3& axis);

	// TODO: Mirroring & Shearing? Local vs. global trafos?

	const std::string& name() const { return this->m_name; }
	uint32_t index_count() const {  return m_geometry.indices.size(); }

private:
	std::string m_name;
	GeometryData m_geometry;
	Material m_material;
};