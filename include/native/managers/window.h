#pragma once

#include <native/core/root.h>
#include <native/util/config.h>

class UserInterface
{
    // TODO
};

class WindowManager
{
public:
    UserInterface ui;

    WindowManager(Root& root) : m_root(root) { }
    void create();
    void destroy();

private:
    Root& m_root;
    GLFWwindow* m_window = nullptr;
    vk::SurfaceKHR m_surface;

    /* === Swapchain === */
    vk::UniqueSwapchainKHR m_swapchain;
    vk::Format m_image_format;
    vk::Extent2D m_extent;

    std::vector<vk::Image> m_images;
    std::vector<vk::UniqueImageView> m_views;

    std::vector<vk::UniqueSemaphore> m_available;
    std::vector<vk::UniqueSemaphore> m_finished;
    std::vector<vk::UniqueFence> m_in_flight;

    vk::SwapchainCreateInfoKHR generate_swapchain_info() const;
    vk::ImageViewCreateInfo generate_imageviewcreate_info(const vk::Image& image) const;
};