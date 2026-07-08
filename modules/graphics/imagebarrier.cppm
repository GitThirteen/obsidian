module;

#include <vulkan/vulkan_raii.hpp>

export module Obsidian.Graphics:ImageBarrier;
import std;

export namespace obsidian
{
    class ImageBarrierBuilder
    {
    public:
        ImageBarrierBuilder(const vk::raii::CommandBuffer& cmd, vk::Image image, vk::ImageAspectFlags aspect, uint32_t mips, uint32_t layers)
            : m_cmd(cmd), m_image(image) 
        {
            m_range = vk::ImageSubresourceRange{aspect, 0, mips, 0, layers};
        }

        ImageBarrierBuilder& from(vk::ImageLayout layout)
        { 
            m_old_layout = layout;
            return *this;
        }

        ImageBarrierBuilder& to(vk::ImageLayout layout)
        { 
            m_new_layout = layout;
            return *this;
        }

        ImageBarrierBuilder& as(vk::AccessFlags access, vk::PipelineStageFlags stage)
        {
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

        void commit()
        {
            vk::ImageMemoryBarrier barrier;
            barrier.setSrcAccessMask(m_src_access)
                   .setDstAccessMask(m_dst_access)
                   .setOldLayout(m_old_layout)
                   .setNewLayout(m_new_layout)
                   .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                   .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                   .setImage(m_image)
                   .setSubresourceRange(m_range);

            m_cmd.pipelineBarrier(
                m_src_stage, m_dst_stage, vk::DependencyFlags{},
                nullptr, nullptr, barrier
            );
        }

    private:
        const vk::raii::CommandBuffer& m_cmd;
        vk::Image m_image;
        vk::ImageSubresourceRange m_range;
        
        vk::ImageLayout m_old_layout = vk::ImageLayout::eUndefined;
        vk::ImageLayout m_new_layout = vk::ImageLayout::eUndefined;
        vk::AccessFlags m_src_access;
        vk::AccessFlags m_dst_access;
        vk::PipelineStageFlags m_src_stage;
        vk::PipelineStageFlags m_dst_stage;
    };

    namespace ImageBarrier
    {
        ImageBarrierBuilder transition(const vk::raii::CommandBuffer& cmd, vk::Image image, vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor, uint32_t mips = 1, uint32_t layers = 1)
        {
            return ImageBarrierBuilder(cmd, image, aspect, mips, layers);
        }
    }
}