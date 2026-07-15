module;
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_raii.hpp>
#include <native/macros.h>

// ============================ //
//   GRAPHICS - WINDOW MODULE   //
// ============================ //

export module Obsidian.Platform.Window;

import std;
import Obsidian.Core.Result;
import Obsidian.Core.Config;
import Obsidian.Core.Logger;
import Obsidian.Magma.Vector;

EXPORT(obsidian)

class WindowManager
{
public:
    explicit WindowManager(const vk::raii::Instance& instance) : m_instance_ptr(&instance) { }
    ~WindowManager() { destroy(); }

    auto initialize() -> Result<void>
    {
        log::info("Initializing window...");

        if (!glfwInit() || !glfwVulkanSupported())
        {
            return std::unexpected<std::string>("Failed to initialize GLFW.");
        }

        const auto name = Config::get<std::string>("title").unwrap_or("Obsidian Engine");
        const auto width = Config::get<int>("dimensions", "width").unwrap_or(1280);
        const auto height = Config::get<int>("dimensions", "height").unwrap_or(920);
        const auto resizable = Config::get<bool>("resizable").unwrap_or(true);

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);

        m_window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
        if (!m_window)
        {
            return std::unexpected<std::string>("Failed to create GLFW window.");
        }

        VkSurfaceKHR raw_surface;
        if (glfwCreateWindowSurface(**m_instance_ptr, m_window, nullptr, &raw_surface) != VK_SUCCESS)
        {
            return std::unexpected<std::string>("Failed to create window surface.");
        }

        m_surface = std::make_unique<vk::raii::SurfaceKHR>(*m_instance_ptr, raw_surface);

        return {};
    }

    auto destroy() -> void
    {
        m_surface.reset();

        if (m_window)
        {
            glfwDestroyWindow(m_window);
        	m_window = nullptr;
        }

        glfwTerminate();
    }

    auto should_close() const -> bool
    {
	    return glfwWindowShouldClose(m_window);
    }

	auto extent() const -> vk::Extent2D
    {
        int width, height;
        glfwGetFramebufferSize(m_window, &width, &height);
    	return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
    }

    auto dimensions() const -> Vector<int, 2>
    {
        int width, height;
        glfwGetFramebufferSize(m_window, &width, &height);
        return Vector<int, 2>(width, height);
    }

    auto handle() const -> GLFWwindow*
    {
	    return m_window;
    }

private:
    GLFWwindow* m_window = nullptr;
    const vk::raii::Instance* m_instance_ptr = nullptr;
    std::unique_ptr<vk::raii::SurfaceKHR> m_surface;
};

EXPORT_END