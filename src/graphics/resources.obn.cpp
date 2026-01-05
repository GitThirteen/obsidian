#include <native/graphics/resources.obn.h>

auto ResourceManager::initialize() -> void
{
	create_swapchain();
	create_pools();
	create_syncers();
	create_shards();
	create_depth_buffer();
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

auto ResourceManager::destroy() -> void
{
	m_root.device().waitIdle();
	m_shards.clear();
	m_swapchain_views.clear();
	m_swapchain_images.clear();
	m_swapchain.reset();
}

auto ResourceManager::next_image(uint32_t frame_index) -> std::pair<bool, uint32_t>
{
	auto wait_result = m_root.device().waitForFences(
		1, &m_in_flight_fences[frame_index].get(), VK_TRUE, UINT64_MAX
	);

	if (wait_result != vk::Result::eSuccess)
	{
		return { false, 0 };
	}

	uint32_t image_index;
	auto acquire_result = m_root.device().acquireNextImageKHR(
		this->m_swapchain.get(), UINT64_MAX, m_image_available_semaphores[frame_index].get(), nullptr, &image_index
	);

	if (acquire_result != vk::Result::eSuccess)
	{
		return { false, 0 };
	}
	
	auto reset_result = m_root.device().resetFences(1, &m_in_flight_fences[frame_index].get());
	
	if (reset_result != vk::Result::eSuccess)
	{
		return { false, 0 };
	}

	return { true, image_index };
}

auto ResourceManager::submit(uint32_t frame_index, uint32_t image_index, avk::command_buffer& cmd) -> void
{
	vk::Semaphore vk_wait_semaphore = this->m_image_available_semaphores[frame_index].get();
	vk::Semaphore vk_signal_semaphore = m_render_finished_semaphores[image_index].get();
	vk::PipelineStageFlags vk_wait_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	vk::SwapchainKHR vk_swapchain = m_swapchain.get();
	vk::Fence fence = m_in_flight_fences[frame_index].get();
	vk::CommandBuffer vk_cmd_buffer = cmd->handle();

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
		{ vk::DescriptorType::eStorageBuffer, 1024 },
		{ vk::DescriptorType::eUniformBuffer, 1024 },
		{ vk::DescriptorType::eCombinedImageSampler, 1024 }
	};

	m_descriptor_pool = m_root.create_descriptor_pool(sizes, 1024);
}

auto ResourceManager::create_shards() -> void
{
	LOG_S(INFO) << "Creating shards...";

	// If no shards are specified, create a default one
	if (!Config::has_values_for("shards"))
	{
		LOG_S(WARNING) << "No shards specified in config, using default instead.";

		Shard default_shard;
		default_shard.attachments.push_back(ShardMetadata::default_color_attachment());
		default_shard.depth_attachment = ShardMetadata::default_depth_attachment();
		default_shard.pipelines.push_back("base");
		default_shard.is_swapchain_target = true;

		m_shards.insert({ "default", default_shard });
		return;
	}

	// Alternatively, create shards as specified in config
	auto shard_configs = Config::get<std::vector<ShardConfig>>("shards");
	for (const auto& shard_config : shard_configs)
	{
		Shard shard;
		shard.pipelines = shard_config.pipelines;
		shard.is_swapchain_target = (shard_config.target == "screen");

		// Parse metadata for each attachment
		for (const auto& attachment_config : shard_config.attachments)
		{
			ShardMetadata metadata;
			metadata.format = Translator::to<vk::Format>(attachment_config.type);
			metadata.load_op = Translator::to<avk::attachment_load_config>(attachment_config.load);
			metadata.store_op = Translator::to<avk::attachment_store_config>(attachment_config.store);
			metadata.usage = Translator::to<avk::subpass_usages>(attachment_config.type);
			metadata.layout = Translator::to<avk::layout::image_layout>(attachment_config.layout);
			metadata.clear_color = shard_config.clear_color;

			if (is_depth_format(metadata.format))
			{
				shard.depth_attachment = metadata;
			}
			else
			{
				shard.attachments.push_back(metadata);
			}
		}

		// If we're not writing to the screen, we must allocate the per-shard off-screen images + views
		if (!shard.is_swapchain_target)
		{
			// TODO: We may need to potentially change the width and height if the config demands so
			auto width = m_window.extent().width;
			auto height = m_window.extent().height;

			for (const auto& meta : shard.attachments)
			{
				auto image = m_root.create_image(width, height, meta.format, 1, avk::memory_usage::device, avk::image_usage::color_attachment | avk::image_usage::sampled);
				auto view = m_root.create_image_view(image);
				image.enable_shared_ownership();

				shard.internal_images.push_back(std::move(image));
				shard.internal_views.push_back(std::move(view));
			}

			if (shard.depth_attachment.format != vk::Format::eUndefined)
			{
				auto depth_image = m_root.create_image(width, height, shard.depth_attachment.format, 1, avk::memory_usage::device, avk::image_usage::depth_stencil_attachment | avk::image_usage::sampled);
				auto depth_view = m_root.create_image_view(depth_image);

				shard.internal_depth_image = std::move(depth_image);
				shard.internal_depth_view = std::move(depth_view);
			}
		}

		m_shards.insert({ shard_config.name, shard });
		LOG_S(INFO) << "Successfully created shard: " << shard_config.name << ".";
	}
}

auto ResourceManager::is_depth_format(vk::Format format) -> bool
{
	return
		format == vk::Format::eD16Unorm ||
		format == vk::Format::eX8D24UnormPack32 ||
		format == vk::Format::eD32Sfloat ||
		format == vk::Format::eS8Uint ||
		format == vk::Format::eD16UnormS8Uint ||
		format == vk::Format::eD24UnormS8Uint ||
		format == vk::Format::eD32SfloatS8Uint;
}