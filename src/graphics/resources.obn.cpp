#include <native/graphics/resources.obn.h>

auto ResourceManager::initialize() -> void
{
	create_swapchain();
	create_pools();
	create_syncers();
	create_renderpass();
	create_depth_buffer();
	create_frame_buffers();
}

auto ResourceManager::create_depth_buffer() -> void
{
	LOG_S(INFO) << "Creating depth buffers...";

	m_depth_image = m_root.create_image(
		m_extent.width, m_extent.height, m_depth_format, 1,
		avk::memory_usage::device, avk::image_usage::depth_stencil_attachment
	);

	m_depth_view = m_root.create_image_view(m_depth_image);
}

auto ResourceManager::create_frame_buffers() -> void
{
	LOG_S(INFO) << "Creating frame buffers...";

	m_framebuffers.clear();

	for (auto& view : m_swapchain_views)
	{
		m_framebuffers.push_back(m_root.create_framebuffer(m_renderpass, view, m_depth_view));
	}
}

auto ResourceManager::create_renderpass() -> void
{
	const avk::renderpass default_pass = m_root.create_renderpass({
		avk::attachment::declare(
			m_color_format,
			avk::on_load::clear.from_previous_layout(avk::layout::undefined),
			avk::usage::color(0),
			avk::on_store::store.in_layout(avk::layout::present_src)
		).set_clear_color({ 1.0f, 0.0f, 1.0f, 1.0f }),
		avk::attachment::declare(
			m_depth_format,
			avk::on_load::clear.from_previous_layout(avk::layout::undefined),
			avk::usage::depth_stencil,
			avk::on_store::dont_care
		)
	});

	m_renderpass = default_pass;
}

auto ResourceManager::destroy() -> void
{
	m_root.device().waitIdle();
	m_framebuffers.clear();
	m_swapchain_views.clear();
	m_swapchain_images.clear();
	m_swapchain.reset();
}

auto ResourceManager::next_image(uint32_t frame_index) -> std::pair<bool, uint32_t>
{
	uint32_t image_index;
	try
	{
		auto result = m_root.device().acquireNextImageKHR(
			this->m_swapchain.get(), UINT64_MAX, m_image_available_semaphores[frame_index].get(), nullptr, &image_index
		);

		m_root.device().resetFences(1, &m_in_flight_fences[frame_index].get());
		return { true, image_index };
	}
	catch (...)
	{
		return { false, 0 };
	}
}

auto ResourceManager::submit(uint32_t frame_index, uint32_t image_index, avk::command_buffer& cmd) -> void
{
	vk::Semaphore vk_wait_semaphore = this->m_image_available_semaphores[image_index].get();
	vk::Semaphore vk_signal_semaphore = m_render_finished_semaphores[image_index].get();
	vk::PipelineStageFlags vk_wait_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	vk::SwapchainKHR vk_swapchain = m_swapchain.get();
	vk::Fence fence = m_in_flight_fences[frame_index].get();
	vk::CommandBuffer vk_cmd_buffer = cmd.get().handle();

	vk::SubmitInfo submit_info{};
	submit_info.setWaitSemaphoreCount(1);
	submit_info.setPWaitSemaphores(&vk_wait_semaphore);
	submit_info.setPWaitDstStageMask(&vk_wait_stage);
	submit_info.setCommandBufferCount(1);
	submit_info.setPCommandBuffers(&vk_cmd_buffer);
	submit_info.setSignalSemaphoreCount(1);
	submit_info.setPSignalSemaphores(&vk_signal_semaphore);

	m_root.queue().handle().submit(submit_info, fence);

	vk::PresentInfoKHR present_info{};
	present_info.setWaitSemaphoreCount(1);
	present_info.setPWaitSemaphores(&vk_signal_semaphore);
	present_info.setSwapchainCount(1);
	present_info.setPSwapchains(&vk_swapchain);
	present_info.setPImageIndices(&image_index);

	try
	{
		auto result = m_root.queue().handle().presentKHR(present_info, m_root.dispatch_loader_core());
		if (result == vk::Result::eSuboptimalKHR)
		{
			LOG_S(WARNING) << "Swapchain suboptimal. Recreate suggested.";
			// TODO: Recreate automatically. Maybe merge with code below.
		}
	}
	catch (const vk::SystemError& error)
	{
		if (error.code() == vk::Result::eErrorOutOfDateKHR)
		{
			LOG_S(ERROR) << "Swapchain errored. Recreate required.";
			// TODO: Recreate automatically.
		}
		else
		{
			LOG_S(ERROR) << "Presentation failure: " << error.what();
			throw;
		}
	}
}

auto ResourceManager::create_swapchain() -> void
{
	LOG_S(INFO) << "Creating swapchain...";

	auto surface_capabilities = m_root.physical_device().getSurfaceCapabilitiesKHR(m_window.surface());
	auto surface_formats = m_root.physical_device().getSurfaceFormatsKHR(m_window.surface());

	if (surface_formats.empty())
	{
		throw std::runtime_error("Physical device has no surface formats to choose from!");
	}

	m_color_format = surface_formats[0].format;
	for (const auto& format : surface_formats)
	{
		if (format.format == OBN_DEFAULT_COLOR_FORMAT && format.colorSpace == OBN_DEFAULT_COLOR_SPACE)
		{
			m_color_format = format.format;
			break;
		}
	}

	m_extent = m_window.extent();
	if (surface_capabilities.currentExtent.width != UINT32_MAX)
	{
		m_extent = surface_capabilities.currentExtent;
	}

	vk::SwapchainCreateInfoKHR info{};
	info.setSurface(m_window.surface());
	info.setMinImageCount(std::max(2u, surface_capabilities.minImageCount + 1));
	info.setImageFormat(m_color_format);
	info.setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear);
	info.setImageExtent(m_extent);
	info.setImageArrayLayers(1);
	info.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);
	info.setPresentMode(vk::PresentModeKHR::eMailbox);
	info.setClipped(VK_TRUE);

	/*info.setSurface(this->m_surface);
	info.setMinImageCount(std::max(2u, caps.minImageCount + 1));
	info.setImageFormat(this->m_image_format);
	info.setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear);
	info.setImageExtent(this->m_extent);
	info.setImageArrayLayers(1);
	info.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);
	info.setImageSharingMode(vk::SharingMode::eExclusive);
	info.setPreTransform(caps.currentTransform);
	info.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
	info.setPresentMode(vk::PresentModeKHR::eMailbox);
	info.setClipped(VK_TRUE);*/

	m_swapchain_images.clear();
	m_swapchain_views.clear();
	m_swapchain = m_root.device().createSwapchainKHRUnique(info);
	auto raw_images = m_root.device().getSwapchainImagesKHR(m_swapchain.get());

	for (const auto& raw_image : raw_images)
	{
		vk::ImageCreateInfo image_info{};
		image_info.setImageType(vk::ImageType::e2D);
		image_info.setFormat(m_color_format);
		image_info.setExtent(vk::Extent3D{ m_extent.width, m_extent.height, 1 });
		image_info.setMipLevels(1);
		image_info.setArrayLayers(1);
		image_info.setSamples(vk::SampleCountFlagBits::e1);
		image_info.setTiling(vk::ImageTiling::eOptimal);
		image_info.setUsage(vk::ImageUsageFlagBits::eColorAttachment);

		/*image_info.setImageType(vk::ImageType::e2D);
		image_info.setFormat(m_image_format);
		image_info.setExtent(vk::Extent3D{ this->m_extent.width, this->m_extent.height, 1 });
		image_info.setMipLevels(1);
		image_info.setArrayLayers(1);
		image_info.setSamples(vk::SampleCountFlagBits::e1);
		image_info.setTiling(vk::ImageTiling::eOptimal);
		image_info.setUsage(vk::ImageUsageFlagBits::eColorAttachment);
		image_info.setSharingMode(vk::SharingMode::eExclusive);
		image_info.setInitialLayout(vk::ImageLayout::eUndefined);*/

		auto wrapped = m_root.wrap_image(
			raw_image, image_info, avk::image_usage::color_attachment, vk::ImageAspectFlagBits::eColor
		);

		auto view = m_root.create_image_view(wrapped);
		m_swapchain_images.push_back(std::move(wrapped));
		m_swapchain_views.push_back(std::move(view));
	}
}

auto ResourceManager::create_syncers() -> void
{
	LOG_S(INFO) << "Creating synchronization...";

	int frames = 2;
	m_image_available_semaphores.resize(frames);
	m_in_flight_fences.resize(frames);

	for (int i = 0; i < frames; ++i)
	{
		m_image_available_semaphores[i] = m_root.device().createSemaphoreUnique({ });
		m_in_flight_fences[i] = m_root.device().createFenceUnique({ vk::FenceCreateFlagBits::eSignaled });
	}

	m_render_finished_semaphores.clear();
	for (size_t i = 0; i < m_swapchain_images.size(); ++i)
	{
		m_render_finished_semaphores.push_back(m_root.device().createSemaphoreUnique({ }));
	}
}

auto ResourceManager::create_pools() -> void
{
	LOG_S(INFO) << "Creating pools...";

	m_command_pool = m_root.create_command_pool(
		m_root.queue().family_index(), vk::CommandPoolCreateFlagBits::eResetCommandBuffer
	);

	std::vector<vk::DescriptorPoolSize> sizes = {
		{ vk::DescriptorType::eStorageBuffer, 1000 },
		{ vk::DescriptorType::eUniformBuffer, 1000 }
	};

	m_descriptor_pool = m_root.create_descriptor_pool(sizes, 1000);
}