module;
#include <vulkan/vulkan_raii.hpp>
export module Obsidian.Graphics:Shard;
import std;
import Obsidian.Core;

export namespace obsidian 
{
    struct ShardMetadata 
    {
        vk::Format format = vk::Format::eUndefined;
        vk::AttachmentLoadOp load_op = vk::AttachmentLoadOp::eDontCare;
        vk::AttachmentStoreOp store_op = vk::AttachmentStoreOp::eDontCare;
        vk::ImageLayout layout = vk::ImageLayout::eUndefined;
        std::array<float, 4> clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };

        static auto default_color_attachment() -> ShardMetadata
        {
            ShardMetadata data;
            data.format = DEFAULT_COLOR_FORMAT;
            data.load_op = vk::AttachmentLoadOp::eClear;
            data.store_op = vk::AttachmentStoreOp::eStore;
            data.layout = vk::ImageLayout::eColorAttachmentOptimal;
            return data;
        }

        static auto default_depth_attachment() -> ShardMetadata
        {
            ShardMetadata data;
            data.format = DEFAULT_DEPTH_FORMAT;
            data.load_op = vk::AttachmentLoadOp::eClear;
            data.store_op = vk::AttachmentStoreOp::eDontCare;
            data.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
            return data;
        }
    };

    struct Shard 
    {
        std::string name;
        std::vector<ShardMetadata> color_attachments;
        ShardMetadata depth_attachment;
        std::vector<std::string> pipelines;
        bool is_swapchain_target = false;
    };
}