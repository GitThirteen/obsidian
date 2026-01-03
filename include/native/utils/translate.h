#pragma once

#include <native/core/include.h>

struct Translator
{
	template <typename From, typename To>
	static auto translate(From from) -> To = delete;

    template <typename To, typename From>
    static auto to(From value) -> To {
        return Translator::translate<From, To>(value);
    }
};

// --------------------------------------------------------------------------
// Specialization: avk::on_store_behavior -> vk::AttachmentStoreOp
// --------------------------------------------------------------------------
template <>
inline auto Translator::translate<avk::on_store_behavior, vk::AttachmentStoreOp>(avk::on_store_behavior value) -> vk::AttachmentStoreOp
{
    switch (value)
    {
    case avk::on_store_behavior::store:     return vk::AttachmentStoreOp::eStore;
    case avk::on_store_behavior::dont_care: return vk::AttachmentStoreOp::eDontCare;
    default:                                return vk::AttachmentStoreOp::eDontCare;
    }
}

// --------------------------------------------------------------------------
// Specialization: avk::on_load_behavior -> vk::AttachmentLoadOp
// --------------------------------------------------------------------------
template <>
inline auto Translator::translate<avk::on_load_behavior, vk::AttachmentLoadOp>(avk::on_load_behavior value) -> vk::AttachmentLoadOp
{
    switch (value)
    {
    case avk::on_load_behavior::load:      return vk::AttachmentLoadOp::eLoad;
    case avk::on_load_behavior::clear:     return vk::AttachmentLoadOp::eClear;
    case avk::on_load_behavior::dont_care: return vk::AttachmentLoadOp::eDontCare;
    default:                               return vk::AttachmentLoadOp::eDontCare;
    }
}

// --------------------------------------------------------------------------
// Specialization: std::string -> various
// --------------------------------------------------------------------------

// 1. String -> vk::Format
template <>
inline auto Translator::translate<std::string, vk::Format>(std::string format) -> vk::Format
{
    if (format == "color") return vk::Format::eR8G8B8A8Unorm; // Or OBN_DEFAULT_COLOR_FORMAT
    if (format == "depth") return vk::Format::eD32Sfloat;     // Or OBN_DEFAULT_DEPTH_FORMAT
    return vk::Format::eUndefined;
}

// 2. String -> avk::attachment_load_config
template <>
inline auto Translator::translate<std::string, avk::attachment_load_config>(std::string load_op) -> avk::attachment_load_config
{
    if (load_op == "clear") return avk::on_load::clear;
    if (load_op == "load")  return avk::on_load::load;
    return avk::on_load::dont_care;
}

// 3. String -> avk::attachment_store_config
template <>
inline auto Translator::translate<std::string, avk::attachment_store_config>(std::string store_op) -> avk::attachment_store_config
{
    if (store_op == "store") return avk::on_store::store;
    return avk::on_store::dont_care;
}

// 4. String -> avk::layout::image_layout
template <>
inline auto Translator::translate<std::string, avk::layout::image_layout>(std::string layout) -> avk::layout::image_layout
{
    if (layout == "present") return avk::layout::present_src;
    if (layout == "color")   return avk::layout::color_attachment_optimal;
    if (layout == "depth")   return avk::layout::depth_stencil_attachment_optimal;
    if (layout == "texture") return avk::layout::shader_read_only_optimal;
    if (layout == "generic") return avk::layout::general;
    return avk::layout::undefined;
}

// 5. String -> avk::subpass_usages (or avk::image_usage depending on version)
template <>
inline auto Translator::translate<std::string, avk::subpass_usages>(std::string usage) -> avk::subpass_usages
{
    if (usage == "color") return avk::usage::color(0);
    if (usage == "depth") return avk::usage::depth_stencil;
    return avk::usage::unused;
}

namespace TranslationDetail {
    template <typename To>
    struct ToWrapper {
        template <typename From>
        static auto from(From val) -> To {
            return Translator::translate<From, To>(val);
        }
    };
}