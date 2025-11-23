#include <native/core/scene/object.h>

SceneObject::SceneObject(Root& root, GeometryData data, std::string name) : m_geometry(std::move(data)), m_name(std::move(name))
{
    m_vertex_buffer = root.create_buffer(
        avk::memory_usage::device,
        vk::BufferUsageFlagBits::eStorageBuffer,
        avk::vertex_buffer_meta::create_from_data(m_geometry.vertices)
    );
    m_vertex_buffer->fill(m_geometry.vertices.data(), 0);

    m_index_buffer = root.create_buffer(
        avk::memory_usage::device,
        vk::BufferUsageFlagBits::eIndexBuffer,
        avk::index_buffer_meta::create_from_data(m_geometry.indices)
    );
    m_index_buffer->fill(m_geometry.indices.data(), 0);
}

void SceneObject::set_position(const glm::vec3& pos)
{
    m_model_matrix[3][0] = pos.x;
    m_model_matrix[3][1] = pos.y;
    m_model_matrix[3][2] = pos.z;
}

void SceneObject::translate(const glm::vec3& offset)
{
    glm::mat4 t = glm::translate(glm::mat4(1.0f), offset);
    m_model_matrix = m_model_matrix * t;
}

void SceneObject::scale(const glm::vec3& factor)
{
    glm::mat4 s = glm::scale(glm::mat4(1.0f), factor);
    m_model_matrix = m_model_matrix * s;
}

void SceneObject::rotate(float angleDeg, const glm::vec3& axis)
{
    glm::mat4 r = glm::rotate(glm::mat4(1.0f), glm::radians(angleDeg), axis);
    m_model_matrix = m_model_matrix * r;
}