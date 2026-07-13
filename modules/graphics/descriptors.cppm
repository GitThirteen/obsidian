module;
#include <vulkan/vulkan_raii.hpp>
#include <native/macros.h>

// =============================== //
//  GRAPHICS - DESCRIPTORS MODULE  //
// =============================== //

export module Obsidian.Graphics:Descriptors;
import std;

EXPORT(obsidian)

auto generate_descriptor_set(vk::DescriptorType type, vk::DescriptorSet set, uint32_t binding) -> vk::WriteDescriptorSet 
{
    vk::WriteDescriptorSet desc_set;
    desc_set.setDstSet(set);
    desc_set.setDstBinding(binding);
    desc_set.setDstArrayElement(0);
    desc_set.setDescriptorCount(1);
    desc_set.setDescriptorType(type);
    return desc_set;
}

auto write_storage_buffer(const vk::raii::Device& device, vk::DescriptorSet set, uint32_t binding, vk::Buffer buffer, vk::DeviceSize offset = 0, vk::DeviceSize range = VK_WHOLE_SIZE) -> void 
{
    vk::DescriptorBufferInfo buffer_info{buffer, offset, range};
    auto desc_set = generate_descriptor_set(vk::DescriptorType::eStorageBuffer, set, binding);
    desc_set.setPBufferInfo(&buffer_info);
    device.updateDescriptorSets(desc_set, nullptr);
}

auto write_uniform_buffer(const vk::raii::Device& device, vk::DescriptorSet set, uint32_t binding, vk::Buffer buffer, vk::DeviceSize offset = 0, vk::DeviceSize range = VK_WHOLE_SIZE) -> void 
{
    vk::DescriptorBufferInfo buffer_info{buffer, offset, range};
    auto desc_set = generate_descriptor_set(vk::DescriptorType::eUniformBuffer, set, binding);
    desc_set.setPBufferInfo(&buffer_info);
    device.updateDescriptorSets(desc_set, nullptr);
}

auto write_combined_image_sampler(const vk::raii::Device& device, vk::DescriptorSet set, uint32_t binding, vk::ImageView image_view, vk::Sampler sampler, vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal) -> void 
{
    vk::DescriptorImageInfo image_info{sampler, image_view, layout};
    auto desc_set = generate_descriptor_set(vk::DescriptorType::eCombinedImageSampler, set, binding);
    desc_set.setPImageInfo(&image_info);
    device.updateDescriptorSets(desc_set, nullptr);
}

EXPORT_END