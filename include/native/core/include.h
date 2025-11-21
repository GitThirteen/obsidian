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

#include <external/avk/avk.hpp>

#define GLFW_INCLUDE_NONE 
#include <external/glfw/glfw3.h>

#ifdef _WIN32
	#pragma comment(lib, "include/external/glfw/glfw3.lib")
	#pragma comment(lib, "gdi32.lib")
	#pragma comment(lib, "user32.lib")
	#pragma comment(lib, "shell32.lib")
#endif

#pragma warning(push, 0)
	#include <external/yaml/ryml.hpp>
	#include <native/core/log.h>
#pragma warning(pop)

template<typename T>
concept IsNumeric = std::integral<T> || std::floating_point<T>;

template<typename T>
concept IsLikeString = std::is_convertible_v<T, std::string_view>;

#define OBSIDIAN_COLOR_FORMAT vk::Format::eB8G8R8A8Unorm
#define OBSIDIAN_DEPTH_FORMAT vk::Format::eD32Sfloat

#endif