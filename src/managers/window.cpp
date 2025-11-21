#include <native/managers/window.h>

void WindowManager::create()
{
    LOG_S(INFO) << "Creating window...";

    auto name = Config::get<std::string>("title");
    auto width = Config::get<int>("dimensions", "width");
    auto height = Config::get<int>("dimensions", "height");
    auto resizable = Config::get<bool>("resizable");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, resizable);

    glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
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
    this->m_images = this->m_root.device().getSwapchainImagesKHR(this->m_swapchain.get());

    for (const auto& image : this->m_images)
    {
        auto view_info = generate_imageviewcreate_info(image);
        this->m_views.push_back(this->m_root.device().createImageViewUnique(view_info));
    }

    int frame_count = 2;
    for (int i = 0; i < frame_count; ++i)
    {
        this->m_available.push_back(this->m_root.device().createSemaphoreUnique({ }));
        this->m_finished.push_back(this->m_root.device().createSemaphoreUnique({ }));
        this->m_in_flight.push_back(this->m_root.device().createFenceUnique({ vk::FenceCreateFlagBits::eSignaled }));
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
