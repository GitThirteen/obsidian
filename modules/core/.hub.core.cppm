module;

#define NOMINMAX
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#define VULKAN_HPP_NO_CONSTRUCTORS
#define WIN32_LEAN_AND_MEAN

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <slang.h>
#include <slang-com-ptr.h> 
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_raii.hpp>

#include <native/macros.h>

// =========================== //
//        CORE MODULE          //
// =========================== //

export module Obsidian.Core;
export import :Logger;
export import :Manager;
export import :Result;
export import std;

EXPORT_OBSIDIAN

template<typename T>
concept IsNumeric = std::integral<T> || std::floating_point<T>;

template<typename T>
concept IsLikeString = std::is_convertible_v<T, std::string_view>;

template<typename T>
concept IsBinaryContainer = std::ranges::contiguous_range<T> && std::ranges::sized_range<T> && std::is_trivially_copyable_v<std::ranges::range_value_t<T>>;

template<typename T>
concept IsVector = std::same_as<T, std::vector<typename T::value_type, typename T::allocator_type>>;

constexpr auto DEFAULT_COLOR_FORMAT = vk::Format::eB8G8R8A8Unorm;
constexpr auto DEFAULT_DEPTH_FORMAT = vk::Format::eD32Sfloat;
constexpr auto DEFAULT_COLOR_SPACE  = vk::ColorSpaceKHR::eSrgbNonlinear;

constexpr double PI       = std::numbers::pi;
constexpr double TWOPI    = std::numbers::pi * 2.0;
constexpr double FOURPI   = std::numbers::pi * 4.0;
constexpr double INVPI    = std::numbers::inv_pi;
constexpr double E        = std::numbers::e;
constexpr double SQRT2    = std::numbers::sqrt2;
constexpr double INVSQRT2 = 0.70710678118654752440;
constexpr double EPSILON  = std::numeric_limits<double>::epsilon();

EXPORT_END