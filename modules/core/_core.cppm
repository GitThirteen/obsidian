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

// =========================== //
//         CORE MODULE         //
// =========================== //

export module Obsidian.Core;

export import Obsidian.Core.Concepts;
export import Obsidian.Core.Config;
export import Obsidian.Core.Logger;
export import Obsidian.Core.Manager;
export import Obsidian.Core.Result;
export import Obsidian.Core.Types;