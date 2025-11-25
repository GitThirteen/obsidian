#pragma once

#include <native/core/root.h>
#include <native/core/config.h>
#include <native/platform/ui.h>

class WindowManager
{
public:
    UserInterface ui;

    WindowManager(Root& root) : m_root(root) { }

    auto create() -> void;
    auto destroy() -> void;
    auto should_close() const -> bool;

    auto glfw_window() const -> GLFWwindow* { return m_window; }
    auto surface() const -> vk::SurfaceKHR { return m_surface; }
    auto extent() const -> vk::Extent2D;

private:
    Root& m_root;
    GLFWwindow* m_window = nullptr;
    vk::SurfaceKHR m_surface;
};