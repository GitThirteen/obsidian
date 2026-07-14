module;
#include <vulkan/vulkan_raii.hpp>
#include <native/macros.h>

// ======================== //
// UTILS - TRANSLATE MODULE //
// ======================== //

export module Obsidian.Utils.Translate;

import std;

NAMESPACE(obsidian, translator, detail)

auto parse_format(std::string_view value) -> vk::Format 
{
    if (value == "color") return vk::Format::eB8G8R8A8Unorm; 
    if (value == "depth") return vk::Format::eD32Sfloat;
    return vk::Format::eUndefined;
}

auto parse_load_op(std::string_view value) -> vk::AttachmentLoadOp 
{
    if (value == "clear") return vk::AttachmentLoadOp::eClear;
    if (value == "load")  return vk::AttachmentLoadOp::eLoad;
    return vk::AttachmentLoadOp::eDontCare;
}

auto parse_store_op(std::string_view value) -> vk::AttachmentStoreOp 
{
    if (value == "store") return vk::AttachmentStoreOp::eStore;
    return vk::AttachmentStoreOp::eDontCare;
}

auto parse_layout(std::string_view value) -> vk::ImageLayout 
{
    if (value == "present") return vk::ImageLayout::ePresentSrcKHR;
    if (value == "color")   return vk::ImageLayout::eColorAttachmentOptimal;
    if (value == "depth")   return vk::ImageLayout::eDepthStencilAttachmentOptimal;
    if (value == "texture") return vk::ImageLayout::eShaderReadOnlyOptimal;
    if (value == "generic") return vk::ImageLayout::eGeneral;
    return vk::ImageLayout::eUndefined;
}

NAMESPACE_END

EXPORT(obsidian, translator)

template <typename To>
auto to(std::string_view value) -> To 
{
    if constexpr (std::is_same_v<To, vk::Format>)                   return detail::parse_format(value);
    if constexpr (std::is_same_v<To, vk::AttachmentLoadOp>)         return detail::parse_load_op(value);
    if constexpr (std::is_same_v<To, vk::AttachmentStoreOp>)        return detail::parse_store_op(value); 
    if constexpr (std::is_same_v<To, vk::ImageLayout>)              return detail::parse_layout(value);
    
    static_assert(sizeof(To) == 0, "Unsupported translation type requested!");
}

EXPORT_END