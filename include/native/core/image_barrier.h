#pragma once

#include <native/core/include.h>

// TODO: This will eventually move into the Magma abstraction layer.

class ImageBarrier
{
public:
    class Builder
    {
    public:
        Builder() = delete;

        auto from(vk::ImageLayout from) -> Builder&;
        auto to(vk::ImageLayout to) -> void;

    private:
        friend class ImageBarrier;

        Builder(vk::CommandBuffer cmd, vk::Image image, vk::ImageAspectFlags aspect_flags, uint32_t mip_levels, uint32_t layers) : m_cmd(cmd), m_image(image)
        {
            m_range.aspectMask = aspect_flags;
            m_range.levelCount = mip_levels;
            m_range.layerCount = layers;
            m_range.baseMipLevel = 0;
            m_range.baseArrayLayer = 0;
        }

        vk::CommandBuffer m_cmd;
        vk::Image m_image;
        vk::ImageSubresourceRange m_range;
        vk::ImageLayout m_old_layout = vk::ImageLayout::eUndefined;
    };

    static auto transition(vk::CommandBuffer cmd, vk::Image image, vk::ImageAspectFlags aspect_flags = vk::ImageAspectFlagBits::eColor, uint32_t mip_levels = 1, uint32_t layers = 1) -> Builder;
};