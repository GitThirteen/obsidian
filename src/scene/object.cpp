#include <native/scene/object.h>

auto SceneObject::create_descriptor_set(Root& root, avk::descriptor_pool& pool, const Pipeline& pipeline) -> void
{
    if (has_descriptor_set()) return;

	m_descriptor_set = pipeline.make_descriptor_set(pool, 1);

    auto bind_texture = [&](int binding, std::shared_ptr<TextureAsset> tex) {
        if (!tex) return;
        ShaderDescriptor::write_combined_image_sampler(root.device(), m_descriptor_set, binding, tex->handle->get_image_view(), tex->handle->get_sampler());
    };

    avk::image_sampler* texture_handle = 
        (m_material->is_volume()) ? &m_material->volume_map->handle 
        : (m_material->albedo_map) ? &m_material->albedo_map->handle 
        : nullptr;

    if (m_material->is_volume())
    {
        bind_texture(1, m_material->volume_map);
    }
    else
    {
        bind_texture(0, m_material->albedo_map);
        bind_texture(1, m_material->roughness_map);
        bind_texture(2, m_material->normal_map);
        bind_texture(3, m_material->ao_map);

        if (m_material->ubo.has_value())
        {
            ShaderDescriptor::write_uniform_buffer(root.device(), m_descriptor_set, 4, m_material->ubo);
        }
    }
}

auto SceneObject::set_position(const glm::vec3& pos) -> void
{
    m_position = pos;
    update_model_matrix();
}

auto SceneObject::translate(const glm::vec3& offset) -> void
{
    m_position += offset;
    update_model_matrix();
}

auto SceneObject::set_rotation(const glm::vec3& euler_angles) -> void
{
    m_rotation = euler_angles;
    update_model_matrix();
}

auto SceneObject::rotate(const glm::vec3& delta_euler_angles) -> void
{
    m_rotation += delta_euler_angles;
    update_model_matrix();
}

auto SceneObject::set_scale(const glm::vec3& factor) -> void
{
    m_scale = factor;
    update_model_matrix();
}

auto SceneObject::scale(const glm::vec3& factor) -> void
{
    m_scale *= factor;
    update_model_matrix();
}

auto SceneObject::update_model_matrix() -> void
{
    glm::mat4 mat(1.0f);

    mat = glm::translate(mat, m_position);

    mat = glm::rotate(mat, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    mat = glm::rotate(mat, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    mat = glm::rotate(mat, glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    mat = glm::scale(mat, m_scale);

    m_model_matrix = mat;
}