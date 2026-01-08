#pragma once

#include <native/core/include.h>
#include <native/core/root.h>

class ResourceManager;
class Root;
struct GLFWwindow;

struct ColorPoint {
    float pos;
    ImVec4 color;
};

struct ScalarPoint {
    float pos;
    float value;
};

struct TransferFunctions {
    std::vector<ColorPoint> particle_colors = { {0.0f, {0,0,0,1}}, {1.0f, {1,1,1,1}} };
    std::vector<ScalarPoint> opacities = { {0.0f, 0.0f}, {1.0f, 1.0f} };
    std::vector<ColorPoint> medium_colors = { {0.0f, {1,1,1,1}}, {1.0f, {1,1,1,1}} };
    std::vector<ScalarPoint> iors = { {0.0f, 1.0f}, {1.0f, 1.33f} };
};

class UserInterface
{
public:
    auto initialize(Root& root, GLFWwindow* window, ResourceManager& resources) const -> void;
    auto render(const vk::CommandBuffer& cb, const vk::ImageView& target_view) -> void;
    auto destroy() -> void;

    auto clear_color() const -> const std::array<float, 4> { return m_clear_color; }

private:
    static auto check_vk_result(VkResult error) -> void;

    template <typename T>
    auto sort_points(std::vector<T>& points) -> void
    {
        std::sort(points.begin(), points.end(), [](const T& a, const T& b) {
            return a.pos < b.pos;
        });
    }

    auto edit_scalar(const char* label, std::vector<ScalarPoint>& points, float min_val, float max_val) -> void;
    auto edit_color(const char* label, std::vector<ColorPoint>& points) -> void;

    std::array<float, 4> m_clear_color = std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 1.00f };
    TransferFunctions tfs;
    static ImGui_ImplVulkanH_Window g_MainWindowData;
};