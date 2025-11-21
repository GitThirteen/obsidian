#pragma once

#include <native/core/include.h>

class Root : public avk::root
{
public:
    vk::Instance instance()
    {
        if (!this->m_instance)
        {
            if constexpr (std::is_same_v<std::remove_cv_t<decltype(this->m_dispatch_loader_core)>, vk::detail::DispatchLoaderDynamic>) {
                reinterpret_cast<vk::detail::DispatchLoaderDynamic*>(&this->m_dispatch_loader_core)->init(vkGetInstanceProcAddr);
            }

            this->m_instance = vk::createInstanceUnique(vk::InstanceCreateInfo{}, nullptr, this->m_dispatch_loader_core);

            if constexpr (std::is_same_v<std::remove_cv_t<decltype(this->m_dispatch_loader_core)>, vk::detail::DispatchLoaderDynamic>) {
                reinterpret_cast<vk::detail::DispatchLoaderDynamic*>(&this->m_dispatch_loader_core)->init(this->m_instance.get());
            }
        }

        return this->m_instance.get();
    }

    vk::PhysicalDevice& physical_device() override
    {
        if (!this->m_physical_device)
        {
            this->m_physical_device = instance().enumeratePhysicalDevices().front();
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
        if (!this->m_device)
        {
            auto queue_family_index = avk::queue::find_best_queue_family_for(physical_device(), {}, avk::queue_selection_preference::versatile_queue, {});
            auto queues = avk::make_vector(avk::queue::prepare(this, 0, 0));
            auto config = avk::queue::get_queue_config_for_DeviceCreateInfo(std::begin(queues), std::end(queues));
            for (auto i = 0; i < std::get<0>(config).size(); ++i)
            {
                std::get<0>(config)[i].setPQueuePriorities(std::get<1>(config)[i].data());
            }

            // Create device
            this->m_device = physical_device().createDeviceUnique(
                vk::DeviceCreateInfo{}
                    .setQueueCreateInfoCount(1u)
                    .setPQueueCreateInfos(std::get<0>(config).data())
            );

            // Assign queue handles
            queues[0].assign_handle();
            this->m_queues = std::move(queues[0]);

            if constexpr (std::is_same_v<std::remove_cv_t<decltype(this->m_dispatch_loader_core)>, vk::detail::DispatchLoaderDynamic>) {
                reinterpret_cast<vk::detail::DispatchLoaderDynamic*>(&this->m_dispatch_loader_core)->init(this->m_device.get());
            }
            if constexpr (std::is_same_v<std::remove_cv_t<decltype(this->m_dispatch_loader_ext)>, vk::detail::DispatchLoaderDynamic>) {
                reinterpret_cast<vk::detail::DispatchLoaderDynamic*>(&this->m_dispatch_loader_ext)->init(this->m_device.get());
            }

            // Create memory allocator
            VmaAllocatorCreateInfo allocator_info = {
                .physicalDevice = physical_device(),
                .device = device(),
                .instance = instance()
            };
            vmaCreateAllocator(&allocator_info, &this->m_memory_allocator);
        }

        return this->m_device.get();
    }

    const vk::Device& device() const override
    {
        assert(this->m_device);
        return this->m_device.get();
    }

    DISPATCH_LOADER_CORE_TYPE& dispatch_loader_core() override
    {
        assert(this->m_dispatch_loader_core);
        return this->m_dispatch_loader_core;
    }

    const DISPATCH_LOADER_CORE_TYPE& dispatch_loader_core() const override
    {
        assert(this->m_dispatch_loader_core);
        return this->m_dispatch_loader_core;
    }

    DISPATCH_LOADER_EXT_TYPE& dispatch_loader_ext() override
    {
        assert(this->m_dispatch_loader_ext);
        return this->m_dispatch_loader_ext;
    }

    const DISPATCH_LOADER_EXT_TYPE& dispatch_loader_ext() const override
    {
        assert(this->m_dispatch_loader_ext);
        return this->m_dispatch_loader_ext;
    }

    vk::Queue queue()
    {
        if (!this->m_device)
        {
            device();
        }

        return this->m_queues.handle();
    }

    AVK_MEM_ALLOCATOR_TYPE& memory_allocator() override
    {
        if (!this->m_device)
        {
            device();
        }

        return this->m_memory_allocator;
    }

    const AVK_MEM_ALLOCATOR_TYPE& memory_allocator() const override
    {
        assert(this->m_memory_allocator);
        return this->m_memory_allocator;
    }

    const avk::renderpass& default_renderpass();

private:
    vk::UniqueHandle<vk::Instance, DISPATCH_LOADER_CORE_TYPE> m_instance;
    vk::PhysicalDevice m_physical_device;
    vk::UniqueHandle<vk::Device, DISPATCH_LOADER_CORE_TYPE> m_device;
    DISPATCH_LOADER_CORE_TYPE m_dispatch_loader_core;
    DISPATCH_LOADER_EXT_TYPE m_dispatch_loader_ext;
    avk::queue m_queues;
    AVK_MEM_ALLOCATOR_TYPE m_memory_allocator;
};