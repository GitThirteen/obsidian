module;
#include <vulkan/vulkan_raii.hpp>
#include <native/macros.h>

// ========================== //
//     CORE - TYPES MODULE    //
// ========================== //

export module Obsidian.Core.Types;

EXPORT(obsidian)

constexpr auto DEFAULT_COLOR_FORMAT = vk::Format::eB8G8R8A8Unorm;
constexpr auto DEFAULT_DEPTH_FORMAT = vk::Format::eD32Sfloat;
constexpr auto DEFAULT_COLOR_SPACE  = vk::ColorSpaceKHR::eSrgbNonlinear;
constexpr auto FRAMES_IN_FLIGHT		= 2;

EXPORT_END