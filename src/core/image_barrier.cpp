#include <native/core/image_barrier.h>

auto ImageBarrier::Builder::from(vk::ImageLayout from) -> Builder&
{
    m_old_layout = from;
    return *this;
}

auto ImageBarrier::Builder::to(vk::ImageLayout to) -> void
{
    vk::AccessFlags src_access;
    vk::AccessFlags dst_access;
    vk::PipelineStageFlags src_stage;
    vk::PipelineStageFlags dst_stage;

    // Undefined -> Transfer Dst
    if (m_old_layout == vk::ImageLayout::eUndefined && to == vk::ImageLayout::eTransferDstOptimal)
    {
        src_access = vk::AccessFlagBits::eNone;
        dst_access = vk::AccessFlagBits::eTransferWrite;
        src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
        dst_stage = vk::PipelineStageFlagBits::eTransfer;
    }
    // Transfer Dst -> Shader Read
    else if (m_old_layout == vk::ImageLayout::eTransferDstOptimal && to == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        src_access = vk::AccessFlagBits::eTransferWrite;
        dst_access = vk::AccessFlagBits::eShaderRead;
        src_stage = vk::PipelineStageFlagBits::eTransfer;
        dst_stage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    // Undefined -> Color Attachment
    else if (m_old_layout == vk::ImageLayout::eUndefined && to == vk::ImageLayout::eColorAttachmentOptimal)
    {
        src_access = vk::AccessFlagBits::eNone;
        dst_access = vk::AccessFlagBits::eColorAttachmentWrite;
        src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
        dst_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    }
    // Color Attachment -> Present
    else if (m_old_layout == vk::ImageLayout::eColorAttachmentOptimal && to == vk::ImageLayout::ePresentSrcKHR)
    {
        src_access = vk::AccessFlagBits::eColorAttachmentWrite;
        dst_access = vk::AccessFlagBits::eNone;
        src_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dst_stage = vk::PipelineStageFlagBits::eBottomOfPipe;
    }
    else
    {
        throw std::invalid_argument("ImageBarrier: Unsupported layout transition combination!");
    }

    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout = m_old_layout;
    barrier.newLayout = to;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_image;
    barrier.subresourceRange = m_range;
    barrier.srcAccessMask = src_access;
    barrier.dstAccessMask = dst_access;

    m_cmd.pipelineBarrier(src_stage, dst_stage, vk::DependencyFlags{}, 0, nullptr, 0, nullptr, 1, &barrier);
}

auto ImageBarrier::transition(vk::CommandBuffer cmd, vk::Image image, vk::ImageAspectFlags aspect, uint32_t mip_levels, uint32_t layers) -> ImageBarrier::Builder
{
    return Builder(cmd, image, aspect, mip_levels, layers);
}