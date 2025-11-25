#include <native/graphics/descriptors.h>

auto ShaderDescriptor::write_storage_buffer(vk::Device device, vk::DescriptorSet set, uint32_t binding, const avk::buffer& buffer, vk::DeviceSize offset, vk::DeviceSize range) -> void
{
    vk::DescriptorBufferInfo buffer_info;
    buffer_info.setBuffer(buffer->handle());
    buffer_info.setOffset(offset);
    buffer_info.setRange(range);

    auto desc_set = generate_descriptor_set(vk::DescriptorType::eStorageBuffer, set, binding);
    desc_set.setPBufferInfo(&buffer_info);

    update_device(device, desc_set);
}

auto ShaderDescriptor::write_uniform_buffer(vk::Device device, vk::DescriptorSet set, uint32_t binding, const avk::buffer& buffer, vk::DeviceSize offset, vk::DeviceSize range) -> void
{
    vk::DescriptorBufferInfo buffer_info;
    buffer_info.setBuffer(buffer->handle());
    buffer_info.setOffset(offset);
    buffer_info.setRange(range);

    auto desc_set = generate_descriptor_set(vk::DescriptorType::eUniformBuffer, set, binding);
    desc_set.setPBufferInfo(&buffer_info);

    update_device(device, desc_set);
}

auto ShaderDescriptor::write_combined_image_sampler(vk::Device device, vk::DescriptorSet set, uint32_t binding, const avk::image_view& image_view, const avk::sampler& sampler, vk::ImageLayout layout) -> void
{
    vk::DescriptorImageInfo image_info;
    image_info.setImageView(image_view->handle());
    image_info.setSampler(sampler->handle());
    image_info.setImageLayout(layout);

    auto desc_set = generate_descriptor_set(vk::DescriptorType::eCombinedImageSampler, set, binding);
    desc_set.setPImageInfo(&image_info);
    
    update_device(device, desc_set);
}

auto ShaderDescriptor::write_storage_image(vk::Device device, vk::DescriptorSet set, uint32_t binding, const avk::image_view& image_view, vk::ImageLayout layout) -> void
{
    vk::DescriptorImageInfo image_info;
    image_info.setImageView(image_view->handle());
    image_info.setImageLayout(layout);

    auto desc_set = generate_descriptor_set(vk::DescriptorType::eStorageImage, set, binding);
    desc_set.setPImageInfo(&image_info);

    update_device(device, desc_set);
}

auto ShaderDescriptor::generate_descriptor_set(vk::DescriptorType type, vk::DescriptorSet set, uint32_t binding) -> vk::WriteDescriptorSet
{
    vk::WriteDescriptorSet desc_set;
    desc_set.setDstSet(set);
    desc_set.setDstBinding(binding);
    desc_set.setDstArrayElement(0);
    desc_set.setDescriptorCount(1);
    desc_set.setDescriptorType(type);

    return desc_set;
}

auto ShaderDescriptor::update_device(vk::Device device, const vk::WriteDescriptorSet& desc_set) -> void
{
    device.updateDescriptorSets(1, &desc_set, 0, nullptr);
}
