#pragma once

#include <native/core/include.h>
#include <native/core/root.h>
#include <native/platform/window.obn.h>

class ResourceManager
{
public:
	ResourceManager(Root& root, WindowManager& window) : m_root(root), m_window(window) { }

	auto initialize() -> void;
	auto create_depth_buffer() -> void;
	auto create_frame_buffers() -> void;
	auto create_renderpass() -> void;
	//void recreate_swapchain();
	auto destroy() -> void;

	auto next_image(uint32_t frame_index) -> std::pair<bool, uint32_t>;
	auto submit(uint32_t frame_index, uint32_t image_index, avk::command_buffer& cmd) -> void;

	auto renderpass() const -> const avk::renderpass { return m_renderpass; }
	auto color_format() const -> const vk::Format { return m_color_format; }
	auto depth_format() const -> const vk::Format { return m_depth_format; }
	auto framebuffer(uint32_t image_index) const -> const avk::framebuffer& { return m_framebuffers[image_index]; }
	auto depth_view() const -> const avk::image_view { return m_depth_view; }
	auto command_pool() -> avk::command_pool& { return m_command_pool; }
	auto descriptor_pool() -> avk::descriptor_pool& { return m_descriptor_pool; }
	auto frames_in_flight() -> const size_t { return m_in_flight_fences.size(); }

private:
	Root& m_root;
	WindowManager& m_window;
	avk::renderpass m_renderpass;

	// Swapchain
	vk::UniqueSwapchainKHR m_swapchain;
	vk::Format m_color_format = OBN_DEFAULT_COLOR_FORMAT;
	vk::Format m_depth_format = OBN_DEFAULT_DEPTH_FORMAT;
	vk::Extent2D m_extent;

	// Buffers
	std::vector<avk::framebuffer> m_framebuffers;
	std::vector<avk::image> m_swapchain_images;
	std::vector<avk::image_view> m_swapchain_views;
	avk::image m_depth_image;
	avk::image_view m_depth_view;

	// Pools
	avk::command_pool m_command_pool;
	avk::descriptor_pool m_descriptor_pool;

	// Syncers
	std::vector<vk::UniqueSemaphore> m_image_available_semaphores;
	std::vector<vk::UniqueSemaphore> m_render_finished_semaphores;
	std::vector<vk::UniqueFence> m_in_flight_fences;

	auto create_swapchain() -> void;
	auto create_syncers() -> void;
	auto create_pools() -> void;
};