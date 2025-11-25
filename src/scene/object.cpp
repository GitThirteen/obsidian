#include <native/scene/object.h>

SceneObject::SceneObject(Root& root, GeometryData data, std::string name) : m_geometry(std::move(data)), m_name(std::move(name))
{
    m_vertex_buffer = root.create_buffer(
        avk::memory_usage::host_visible,
        vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
        avk::vertex_buffer_meta::create_from_data(m_geometry.vertices)
    );
    m_vertex_buffer->fill(m_geometry.vertices.data(), 0);

    m_index_buffer = root.create_buffer(
        avk::memory_usage::host_visible,
        vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        avk::index_buffer_meta::create_from_data(m_geometry.indices)
    );
    m_index_buffer->fill(m_geometry.indices.data(), 0);
}

auto SceneObject::create_descriptor_set(avk::descriptor_pool& pool, avk::graphics_pipeline& pipeline, Root& root) -> void
{
    if (has_descriptor_set()) return;

    auto allocated = pool.allocate({
        pipeline->descriptor_set_layouts().set_at(0)
    });

    m_descriptor_set = allocated[0];
    ShaderDescriptor::write_storage_buffer(root.device(), m_descriptor_set, 0, m_vertex_buffer);
}

auto SceneObject::set_position(const glm::vec3& pos) -> void
{
    m_model_matrix[3][0] = pos.x;
    m_model_matrix[3][1] = pos.y;
    m_model_matrix[3][2] = pos.z;
}

auto SceneObject::translate(const glm::vec3& offset) -> void
{
    glm::mat4 t = glm::translate(glm::mat4(1.0f), offset);
    m_model_matrix = m_model_matrix * t;
}

auto SceneObject::scale(const glm::vec3& factor) -> void
{
    glm::mat4 s = glm::scale(glm::mat4(1.0f), factor);
    m_model_matrix = m_model_matrix * s;
}

auto SceneObject::rotate(float angleDeg, const glm::vec3& axis) -> void
{
    glm::mat4 r = glm::rotate(glm::mat4(1.0f), glm::radians(angleDeg), axis);
    m_model_matrix = m_model_matrix * r;
}