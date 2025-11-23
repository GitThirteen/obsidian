#pragma once

#include <native/core/obsidian/root.h>
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
    bool should_close();
    void wait_for_fence(int frame_index);
    void reset_fence(int frame_index);
    vk::Fence get_fence(int frame_index) const;
    std::pair<bool, uint32_t> acquire_next_image(int frame_index);
    GLFWwindow* get_window();
    avk::image_view get_current_view(uint32_t image_index);
    const avk::framebuffer& get_current_framebuffer(uint32_t image_index);
    void present(int frame_index, uint32_t image_index, vk::Queue queue);
    void destroy();

    vk::Semaphore get_image_available_semaphore(int frame_index) const { return this->m_available[frame_index].get(); }
    vk::Semaphore get_render_finished_semaphore(int image_index) const { return this->m_finished[image_index].get(); }
    size_t in_flight_frame_count() const { return this->m_in_flight.size(); }

private:
    Root& m_root;
    GLFWwindow* m_window = nullptr;
    vk::SurfaceKHR m_surface;

    /* === Swapchain === */
    vk::UniqueSwapchainKHR m_swapchain;
    vk::Format m_image_format;
    vk::Extent2D m_extent;

    std::vector<avk::image> m_images;
    std::vector<avk::image_view> m_views;
    std::vector<avk::framebuffer> m_framebuffers;

    avk::image m_depth_image;
    avk::image_view m_depth_view;

    /* === Locks === */
    std::vector<vk::UniqueSemaphore> m_available;
    std::vector<vk::UniqueSemaphore> m_finished;
    std::vector<vk::UniqueFence> m_in_flight;

    vk::SwapchainCreateInfoKHR generate_swapchain_info() const;
    vk::ImageViewCreateInfo generate_imageviewcreate_info(const vk::Image& image) const;
    vk::ImageCreateInfo generate_imagecreate_info() const;
    void create_depth_resources();
};