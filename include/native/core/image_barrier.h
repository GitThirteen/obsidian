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

        // Sets the old layout and returns the builder for chaining.
        auto from(vk::ImageLayout layout) -> Builder&;

        // Sets the new layout and returns the builder.
        auto to(vk::ImageLayout layout) -> Builder&;

        // Specifies the access and pipeline stage flags.
        auto as(vk::AccessFlags access, vk::PipelineStageFlags stage) -> Builder&;

        // Finalizes the barrier.
        auto commit() -> void;

    private:
        friend class ImageBarrier;

        Builder(vk::CommandBuffer cmd, vk::Image image, vk::ImageAspectFlags aspect_flags, uint32_t mip_levels, uint32_t layers);

        vk::CommandBuffer m_cmd;
        vk::Image m_image;
        vk::ImageSubresourceRange m_range;

        vk::ImageLayout m_old_layout = vk::ImageLayout::eUndefined;
        vk::ImageLayout m_new_layout = vk::ImageLayout::eUndefined;

        vk::AccessFlags m_src_access = vk::AccessFlagBits::eNone;
        vk::PipelineStageFlags m_src_stage = vk::PipelineStageFlagBits::eTopOfPipe;

        vk::AccessFlags m_dst_access = vk::AccessFlagBits::eNone;
        vk::PipelineStageFlags m_dst_stage = vk::PipelineStageFlagBits::eTopOfPipe;
    };

    static auto transition(vk::CommandBuffer cmd, vk::Image image, vk::ImageAspectFlags aspect_flags = vk::ImageAspectFlagBits::eColor, uint32_t mip_levels = 1, uint32_t layers = 1) -> Builder;
};