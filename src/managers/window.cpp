#include <native/managers/window.h>

void WindowManager::create()
{
    LOG_S(INFO) << "Creating window...";

    if (!glfwInit() || !glfwVulkanSupported())
    {
        throw std::runtime_error("Failed to initialize GLFW.");
    }

    auto name = Config::get<std::string>("title");
    auto width = Config::get<int>("dimensions", "width");
    auto height = Config::get<int>("dimensions", "height");
    auto resizable = Config::get<bool>("resizable");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, resizable);

    this->m_window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
    if (!this->m_window)
    {
        throw std::runtime_error("Failed to create GLFW window.");
    }

    VkSurfaceKHR raw_surface;
    auto res = glfwCreateWindowSurface(this->m_root.instance(), this->m_window, nullptr, &raw_surface);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create window surface.");
    }

    this->m_surface = raw_surface;
    auto formats = this->m_root.physical_device().getSurfaceFormatsKHR(this->m_surface);
    this->m_image_format = formats[0].format;

    for (const auto& format : formats)
    {
        if (format.format == OBSIDIAN_COLOR_FORMAT && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            this->m_image_format = format.format;
            break;
        }
    }

    this->m_extent = vk::Extent2D{ (uint32_t) width, (uint32_t) height };

    LOG_S(INFO) << "Creating swapchain...";

    vk::SwapchainCreateInfoKHR info = generate_swapchain_info();
    this->m_swapchain = this->m_root.device().createSwapchainKHRUnique(info);
    auto raw_images = this->m_root.device().getSwapchainImagesKHR(this->m_swapchain.get());

    this->m_images.reserve(raw_images.size());
    this->m_views.reserve(raw_images.size());

    for (const auto& raw_image : raw_images)
    {
        auto wrapped_image = this->m_root.wrap_image(
            raw_image,
            generate_imagecreate_info(),
            avk::image_usage::color_attachment,
            vk::ImageAspectFlagBits::eColor
        );

        auto view = this->m_root.create_image_view(wrapped_image);

        this->m_images.push_back(std::move(wrapped_image));
        this->m_views.push_back(std::move(view));
    }

    for (int i = 0; i < this->m_images.size(); ++i)
    {
        this->m_finished.push_back(this->m_root.device().createSemaphoreUnique({ }));
    }

    int frame_count = 2;
    for (int i = 0; i < frame_count; ++i)
    {
        this->m_available.push_back(this->m_root.device().createSemaphoreUnique({ }));
        this->m_in_flight.push_back(this->m_root.device().createFenceUnique({ vk::FenceCreateFlagBits::eSignaled }));
    }

    m_framebuffers.reserve(this->m_views.size());

    create_depth_resources();

    for (auto& view : this->m_views)
    {
        auto framebuffer = this->m_root.create_framebuffer(this->m_root.default_renderpass(), view, this->m_depth_view);
        m_framebuffers.push_back(std::move(framebuffer));
    }
}

bool WindowManager::should_close()
{
    return glfwWindowShouldClose(this->m_window);
}

void WindowManager::wait_for_fence(int frame_index)
{
    auto& fence = this->m_in_flight[frame_index];
    if (this->m_root.device().waitForFences(1, &fence.get(), VK_TRUE, UINT64_MAX) != vk::Result::eSuccess)
    {
        LOG_S(ERROR) << "Waiting for fence failed!";
    }
}

void WindowManager::reset_fence(int frame_index)
{
    auto result = this->m_root.device().resetFences(1, &this->m_in_flight[frame_index].get());
    if (result != vk::Result::eSuccess)
    {
        LOG_S(ERROR) << "Resetting fence failed!";
    }
}

vk::Fence WindowManager::get_fence(int frame_index) const
{
    return this->m_in_flight[frame_index].get();
}

std::pair<bool, uint32_t> WindowManager::acquire_next_image(int frame_index)
{
    uint32_t image_index;
    try
    {
        auto result = this->m_root.device().acquireNextImageKHR(
            this->m_swapchain.get(), UINT64_MAX, this->m_available[frame_index].get(), nullptr, &image_index
        );

        return { true, image_index };
    }
    catch (...)
    {
        return { false, 0 }; // TODO: proper error handling?
    }
}

avk::image_view WindowManager::get_current_view(uint32_t image_index)
{
    return this->m_views[image_index];
}

const avk::framebuffer& WindowManager::get_current_framebuffer(uint32_t image_index)
{
    return this->m_framebuffers[image_index];
}

GLFWwindow* WindowManager::get_window()
{
    return this->m_window;
}

void WindowManager::present(int frame_index, uint32_t image_index, vk::Queue queue)
{
    vk::Semaphore wait_semaphores[] = { this->m_finished[image_index].get() };
    vk::SwapchainKHR swapchains[] = { this->m_swapchain.get() };

    vk::PresentInfoKHR present_info;
    present_info.setWaitSemaphoreCount(1);
    present_info.setSwapchainCount(1);
    present_info.setPWaitSemaphores(wait_semaphores);
    present_info.setPSwapchains(swapchains);
    present_info.setPImageIndices(&image_index);

    try
    {
        auto result = queue.presentKHR(present_info, this->m_root.dispatch_loader_core());
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

void WindowManager::destroy()
{
    this->m_root.device().waitIdle();
    this->m_root.instance().destroySurfaceKHR(this->m_surface);
    
    glfwDestroyWindow(this->m_window);
    glfwTerminate();
}

vk::SwapchainCreateInfoKHR WindowManager::generate_swapchain_info() const
{
    auto caps = this->m_root.physical_device().getSurfaceCapabilitiesKHR(this->m_surface);

    vk::SwapchainCreateInfoKHR create_info;
    create_info.setSurface(this->m_surface);
    create_info.setMinImageCount(std::max(2u, caps.minImageCount + 1));
    create_info.setImageFormat(this->m_image_format);
    create_info.setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear);
    create_info.setImageExtent(this->m_extent);
    create_info.setImageArrayLayers(1);
    create_info.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);
    create_info.setImageSharingMode(vk::SharingMode::eExclusive);
    create_info.setPreTransform(caps.currentTransform);
    create_info.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
    create_info.setPresentMode(vk::PresentModeKHR::eMailbox);
    create_info.setClipped(VK_TRUE);

    return create_info;
}

vk::ImageViewCreateInfo WindowManager::generate_imageviewcreate_info(const vk::Image& image) const
{
    vk::ImageViewCreateInfo view_info;
    view_info.setImage(image);
    view_info.setViewType(vk::ImageViewType::e2D);
    view_info.setFormat(this->m_image_format);
    view_info.setSubresourceRange({ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });

    return view_info;
}

vk::ImageCreateInfo WindowManager::generate_imagecreate_info() const
{
    vk::ImageCreateInfo image_info;
    image_info.setImageType(vk::ImageType::e2D);
    image_info.setFormat(m_image_format);
    image_info.setExtent(vk::Extent3D{ this->m_extent.width, this->m_extent.height, 1 });
    image_info.setMipLevels(1);
    image_info.setArrayLayers(1);
    image_info.setSamples(vk::SampleCountFlagBits::e1);
    image_info.setTiling(vk::ImageTiling::eOptimal);
    image_info.setUsage(vk::ImageUsageFlagBits::eColorAttachment);
    image_info.setSharingMode(vk::SharingMode::eExclusive);
    image_info.setInitialLayout(vk::ImageLayout::eUndefined);

    return image_info;
}

void WindowManager::create_depth_resources()
{
    this->m_depth_image = this->m_root.create_image(
        this->m_extent.width,
        this->m_extent.height,
        OBSIDIAN_DEPTH_FORMAT,
        1,
        avk::memory_usage::device,
        avk::image_usage::depth_stencil_attachment
    );

    this->m_depth_view = this->m_root.create_image_view(this->m_depth_image);
}