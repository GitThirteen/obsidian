#include <native/platform/window.obn.h>

auto WindowManager::create() -> void
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

    m_window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
    if (!m_window)
    {
        throw std::runtime_error("Failed to create GLFW window.");
    }

    VkSurfaceKHR raw_surface;
    auto res = glfwCreateWindowSurface(m_root.instance(), m_window, nullptr, &raw_surface);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create window surface.");
    }

    m_surface = raw_surface;
}

auto WindowManager::destroy() -> void
{
    if (m_surface)
    {
        m_root.instance().destroySurfaceKHR(m_surface);
    }

    if (m_window)
    {
        glfwDestroyWindow(m_window);
    }
    
    glfwTerminate();
}

auto WindowManager::should_close() const -> bool
{
    return glfwWindowShouldClose(m_window);
}

auto WindowManager::extent() const -> vk::Extent2D
{
    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
    return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
}