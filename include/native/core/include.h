#pragma once

#ifndef OBSIDIAN_INCLUDES
#define OBSIDIAN_INCLUDES

#ifndef AVK_USE_VMA
	#define AVK_USE_VMA
#endif

#ifdef _WIN32
	#define NOMINMAX
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
#endif

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <map>
#include <cmath>
#include <algorithm>
#include <random>
#include <functional>
#include <vector>
#include <array>
#include <ctime>
#include <chrono>
#include <bit>
#include <numeric>
#include <tuple>
#include <concepts>
#include <span>
#include <thread>
#include <variant>

#include <external/avk/avk.hpp>

#define GLFW_INCLUDE_NONE
#include <external/glfw/glfw3.h>
#include <external/glm/glm.hpp>
#include <external/glm/gtc/matrix_transform.hpp>
#include <external/stbimage/stb_image.h>
#include <external/tinyobjloader/tiny_obj_loader.h>

#ifdef _WIN32
	// GLFW and windows stuff
	#pragma comment(lib, "include/external/glfw/glfw3.lib")
	#pragma comment(lib, "gdi32.lib")
	#pragma comment(lib, "user32.lib")
	#pragma comment(lib, "shell32.lib")
	// GLSL to SPIRV support
	#pragma comment(lib, "glslang.lib")
	#pragma comment(lib, "glslang-default-resource-limits.lib")
	#pragma comment(lib, "SPIRV.lib")
	#pragma comment(lib, "SPIRV-Tools.lib")
	#pragma comment(lib, "SPIRV-Tools-opt.lib")
	#pragma comment(lib, "OSDependent.lib")
	#pragma comment(lib, "MachineIndependent.lib")
	#pragma comment(lib, "GenericCodeGen.lib")
#endif

#pragma warning(push, 0)
	#include <native/core/log.h>
	#include <external/yaml/ryml.hpp>
	#include <external/glslang/SPIRV/GlslangToSpv.h>
	#include <external/glslang/Public/ShaderLang.h>
	#include <external/reflect/spirv_reflect.h>
#pragma warning(pop)

template<typename T>
concept IsNumeric = std::integral<T> || std::floating_point<T>;

template<typename T>
concept IsLikeString = std::is_convertible_v<T, std::string_view>;

template<typename T>
concept IsValidPipeline = std::same_as<T, avk::graphics_pipeline> || std::same_as<T, avk::ray_tracing_pipeline> || std::same_as<T, avk::compute_pipeline>;

template<typename T>
concept IsBinaryContainer = std::ranges::contiguous_range<T> && std::ranges::sized_range<T> && std::is_trivially_copyable_v<std::ranges::range_value_t<T>>;

template<typename T>
concept IsVector = std::same_as<T, std::vector<typename T::value_type, typename T::allocator_type>>;

#define OBN_DEFAULT_COLOR_FORMAT vk::Format::eB8G8R8A8Unorm
#define OBN_DEFAULT_DEPTH_FORMAT vk::Format::eD32Sfloat
#define OBN_DEFAULT_COLOR_SPACE  vk::ColorSpaceKHR::eSrgbNonlinear

#define OBN_PI			3.14159265358979323846
#define OBN_TWOPI		6.28318530717958647692
#define OBN_FOURPI		12.5663706143591729538
#define OBN_INVPI		0.31830988618379067154
#define OBN_E			2.71828182845904523536
#define OBN_SQRT2		1.41421356237309504880
#define OBN_INVSQRT2	0.70710678118654752440

#define OBN_EPSILON		0.00001

#endif