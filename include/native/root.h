#pragma once

#include <./avk/avk.hpp>

class Root : avk::root
{
public:
    vk::PhysicalDevice& physical_device() override
    {
        if (!this->m_physical_device)
        {

        }

        return this->m_physical_device;
    }

    const vk::PhysicalDevice& physical_device() const override
    {
        assert(this->m_physical_device);
        return this->m_physical_device;
    }

    vk::Device& device() override
    {

    }

    const vk::Device& device() const override
    {

    }

    DISPATCH_LOADER_CORE_TYPE& dispatch_loader_core() override
    {

    }

    const DISPATCH_LOADER_CORE_TYPE& dispatch_loader_core() const override
    {

    }

    DISPATCH_LOADER_EXT_TYPE& dispatch_loader_ext() override
    {

    }

    const DISPATCH_LOADER_EXT_TYPE& dispatch_loader_ext() const override
    {

    }

    AVK_MEM_ALLOCATOR_TYPE& memory_allocator() override
    {

    }

    const AVK_MEM_ALLOCATOR_TYPE& memory_allocator() const override
    {

    }


private:
    vk::UniqueHandle<vk::Instance, DISPATCH_LOADER_CORE_TYPE> m_instance;
    vk::PhysicalDevice m_physical_device;
    vk::UniqueHandle<vk::Device, DISPATCH_LOADER_CORE_TYPE> m_device;
    DISPATCH_LOADER_CORE_TYPE m_dispatch_loader_core;
    DISPATCH_LOADER_EXT_TYPE m_dispatch_loader_ext;
    std::array<avk::queue, 2> m_queues;
    AVK_MEM_ALLOCATOR_TYPE m_memory_allocator;
};