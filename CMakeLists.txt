cmake_minimum_required(VERSION 3.28)

if(CMAKE_VERSION VERSION_GREATER_EQUAL "4.4.0")
    set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "f35a9ac6-8463-4d38-8eec-5d6008153e7d")
elseif(CMAKE_VERSION VERSION_GREATER_EQUAL "4.3.0")
    set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
elseif(CMAKE_VERSION VERSION_GREATER_EQUAL "4.0.3")
    set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
elseif(CMAKE_VERSION VERSION_GREATER_EQUAL "4.0.0")
    set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "a9e1cf81-9932-4810-974b-6eccaf14e457")
elseif(CMAKE_VERSION VERSION_GREATER_EQUAL "3.31.8")
    set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
elseif(CMAKE_VERSION VERSION_GREATER_EQUAL "3.30.0")
    set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")
endif()

project(ObsidianRenderer CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_MODULE_STD ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /arch:AVX2")
endif()

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported)

find_package(Vulkan REQUIRED)

add_executable(Obsidian)

if(ipo_supported)
    set_property(TARGET Obsidian PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
endif()

target_compile_options(Obsidian PRIVATE
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic -Wno-unused-command-line-argument>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX->
)

include(FetchContent)

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(glm)

# VulkanMemoryAllocator (VMA)
FetchContent_Declare(
    vma
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG        v3.4.0
)
FetchContent_MakeAvailable(vma)

# Khronos SLANG
FetchContent_Declare(
    slang
    GIT_REPOSITORY https://github.com/shader-slang/slang.git
    GIT_TAG        master
)
set(SLANG_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(SLANG_ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SLANG_ENABLE_GFX OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(slang)

# ImGui (UI)
FetchContent_Declare(
    obn_imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        master
)
if(POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
endif()
FetchContent_GetProperties(obn_imgui)
if(NOT obn_imgui_POPULATED)
    FetchContent_Populate(obn_imgui)
endif()
add_library(obn_imgui STATIC
    ${obn_imgui_SOURCE_DIR}/imgui.cpp
    ${obn_imgui_SOURCE_DIR}/imgui_demo.cpp
    ${obn_imgui_SOURCE_DIR}/imgui_draw.cpp
    ${obn_imgui_SOURCE_DIR}/imgui_tables.cpp
    ${obn_imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${obn_imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${obn_imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
)
target_include_directories(obn_imgui PUBLIC 
    ${obn_imgui_SOURCE_DIR} 
    ${obn_imgui_SOURCE_DIR}/backends
)
target_link_libraries(obn_imgui PRIVATE Vulkan::Vulkan glfw)

file(GLOB_RECURSE MODULE_FILES CONFIGURE_DEPENDS "modules/*.cppm")
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "src/*.cpp")

target_sources(Obsidian
    PUBLIC
        FILE_SET CXX_MODULES FILES ${MODULE_FILES}
    PRIVATE
        ${SRC_FILES}
)

set_target_properties(Obsidian PROPERTIES CXX_SCAN_FOR_MODULES ON)

target_include_directories(Obsidian PRIVATE include)

target_link_libraries(Obsidian PRIVATE 
    Vulkan::Vulkan 
    glfw 
    glm::glm 
    VulkanMemoryAllocator 
    slang 
    obn_imgui
)