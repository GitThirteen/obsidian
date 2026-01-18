#include <native/core/image_barrier.h>

ImageBarrier::Builder::Builder(vk::CommandBuffer cmd, vk::Image image, vk::ImageAspectFlags aspect_flags, uint32_t mip_levels, uint32_t layers)
    : m_cmd(cmd), m_image(image)
{
    m_range.aspectMask = aspect_flags;
    m_range.levelCount = mip_levels;
    m_range.layerCount = layers;
    m_range.baseMipLevel = 0;
    m_range.baseArrayLayer = 0;
}

auto ImageBarrier::Builder::from(vk::ImageLayout layout) -> Builder&
{
    m_old_layout = layout;
    return *this;
}

auto ImageBarrier::Builder::to(vk::ImageLayout layout) -> Builder&
{
    m_new_layout = layout;
    return *this;
}

auto ImageBarrier::Builder::as(vk::AccessFlags access, vk::PipelineStageFlags stage) -> Builder&
{
    // Idea: If 'to' hasn't been called yet, we are configuring the source side. Otherwise, we configure the destination side.
    if (m_new_layout == vk::ImageLayout::eUndefined)
    {
        m_src_access = access;
        m_src_stage = stage;
    }
    else
    {
        m_dst_access = access;
        m_dst_stage = stage;
    }

    return *this;
}

auto ImageBarrier::Builder::commit() -> void
{
    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout = m_old_layout;
    barrier.newLayout = m_new_layout;
    barrier.srcAccessMask = m_src_access;
    barrier.dstAccessMask = m_dst_access;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_image;
    barrier.subresourceRange = m_range;

    m_cmd.pipelineBarrier(
        m_src_stage,
        m_dst_stage,
        vk::DependencyFlags{},
        0, nullptr, 0, nullptr, 1, &barrier
    );
}

auto ImageBarrier::transition(vk::CommandBuffer cmd, vk::Image image, vk::ImageAspectFlags aspect, uint32_t mips, uint32_t layers) -> ImageBarrier::Builder
{
    return Builder(cmd, image, aspect, mips, layers);
}