#pragma once

#include <native/core/root.h>

struct ShaderDescriptor
{
	/**
	 * @brief Writes a Storage Buffer (SSBO) to the descriptor set.
	 * Use this for large, read/write buffers like Vertex Pulling buffers,
	 * Particle System data, or large arrays of object data.
	 * Corresponds to: layout(binding=X) buffer ...
	 */
	static auto write_storage_buffer(
		vk::Device device, 
		vk::DescriptorSet set, 
		uint32_t binding, 
		const avk::buffer& buffer, 
		vk::DeviceSize offset = 0, 
		vk::DeviceSize range = VK_WHOLE_SIZE
	) -> void;

	/**
	 * @brief Writes a Uniform Buffer (UBO) to the descriptor set.
	 * Use this for small, read-only, high-speed data like Camera Matrices (View/Proj),
	 * Light parameters, or Global Scene settings.
	 * Corresponds to: layout(binding=X) uniform ...
	 */
	static auto write_uniform_buffer(
		vk::Device device,
		vk::DescriptorSet set,
		uint32_t binding,
		const avk::buffer& buffer,
		vk::DeviceSize offset = 0,
		vk::DeviceSize range = VK_WHOLE_SIZE
	) -> void;

	/**
	 * @brief Writes a Texture + Sampler (Combined) to the descriptor set.
	 * Use this for standard texturing where the shader samples an image using UVs.
	 * Examples: Albedo maps, Normal maps, Roughness maps, or Skyboxes.
	 * Corresponds to: layout(binding=X) uniform sampler2D ...
	 */
	static auto write_combined_image_sampler(
		vk::Device device,
		vk::DescriptorSet set,
		uint32_t binding,
		const avk::image_view& image_view,
		const avk::sampler& sampler,
		vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal
	) -> void;

	/**
	 * @brief Writes a Storage Image (Read/Write Texture) to the descriptor set.
	 * Use this when the shader needs to write directly to specific pixels,
	 * such as Compute Shader outputs, Ray Tracing targets, or image processing effects.
	 * Corresponds to: layout(binding=X, rgba8) uniform image2D ...
	 */
	static auto write_storage_image(
		vk::Device device,
		vk::DescriptorSet set,
		uint32_t binding,
		const avk::image_view& image_view,
		vk::ImageLayout layout = vk::ImageLayout::eGeneral
	) -> void;

private:
	// Important: Don't forget to set a setPImageInfo or setPBufferInfo after returning this!
	static auto generate_descriptor_set(vk::DescriptorType type, vk::DescriptorSet set, uint32_t binding) -> vk::WriteDescriptorSet;
	static auto	update_device(vk::Device device, const vk::WriteDescriptorSet& desc) -> void;
};