#pragma once

#include <native/core/obsidian/include.h>
#include <native/util/config.h>

class Root : public avk::root
{
public:
    Root()
    {
        initialize();
    }

    void initialize()
    {
        if (m_initialized) return;

        // --- A. Initialize Global Dispatcher ---
        if constexpr (std::is_same_v<std::remove_cv_t<decltype(m_dispatch_loader_core)>, vk::detail::DispatchLoaderDynamic>) {
            reinterpret_cast<vk::detail::DispatchLoaderDynamic*>(&m_dispatch_loader_core)->init(vkGetInstanceProcAddr);
        }
        if constexpr (std::is_same_v<std::remove_cv_t<decltype(m_dispatch_loader_ext)>, vk::detail::DispatchLoaderDynamic>) {
            reinterpret_cast<vk::detail::DispatchLoaderDynamic*>(&m_dispatch_loader_ext)->init(vkGetInstanceProcAddr);
        }

        // --- B. Create Instance ---
        std::vector<const char*> layers = { "VK_LAYER_KHRONOS_validation" };
        std::vector<const char*> extensions = {
            VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef _WIN32
            "VK_KHR_win32_surface",
#endif
#ifdef __APPLE__
            "VK_MVK_macos_surface",
#endif
#ifdef __linux
            "VK_KHR_xcb_surface",
#endif
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME
        };

        const std::string& app_name = Config::get<std::string>("title");
        VkApplicationInfo app_info = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = app_name.c_str(),
            .applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
            .pEngineName = app_name.c_str(),
            .engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
            .apiVersion = VK_API_VERSION_1_3
        };

        VkInstanceCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &app_info,
            .enabledLayerCount = (uint32_t)layers.size(),
            .ppEnabledLayerNames = layers.data(),
            .enabledExtensionCount = (uint32_t)extensions.size(),
            .ppEnabledExtensionNames = extensions.data()
        };

        m_instance = vk::createInstanceUnique(create_info, nullptr, m_dispatch_loader_core);
        if (!m_instance) {
            LOG_S(ERROR) << "Instance creation failed!";
            throw std::runtime_error("Vulkan Init Failed");
        }

        // --- C. Initialize Instance Dispatchers ---
        if constexpr (std::is_same_v<std::remove_cv_t<decltype(m_dispatch_loader_core)>, vk::detail::DispatchLoaderDynamic>) {
            reinterpret_cast<vk::detail::DispatchLoaderDynamic*>(&m_dispatch_loader_core)->init(m_instance.get());
        }
        if constexpr (std::is_same_v<std::remove_cv_t<decltype(m_dispatch_loader_ext)>, vk::detail::DispatchLoaderDynamic>) {
            reinterpret_cast<vk::detail::DispatchLoaderDynamic*>(&m_dispatch_loader_ext)->init(m_instance.get());
        }

        create_debug_callback();

        // --- D. Pick Physical Device ---
        m_physical_device = instance().enumeratePhysicalDevices().front();

        // --- E. Find Queue Family ---
        auto candidates = avk::queue::find_best_queue_family_for(physical_device(), {}, avk::queue_selection_preference::versatile_queue, {});
        if (candidates.empty()) {
            throw std::runtime_error("No suitable queue family found!");
        }
        uint32_t family_index = std::get<0>(candidates.front());

        // --- F. Create Logical Device ---
        auto queues = avk::make_vector(avk::queue::prepare(this, family_index, 0));
        auto config = avk::queue::get_queue_config_for_DeviceCreateInfo(std::begin(queues), std::end(queues));

        for (auto i = 0; i < std::get<0>(config).size(); ++i) {
            std::get<0>(config)[i].setPQueuePriorities(std::get<1>(config)[i].data());
        }

        const std::vector<const char*> device_extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME
        };

        vk::DeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.setQueueCreateInfoCount(1u);
        deviceCreateInfo.setPQueueCreateInfos(std::get<0>(config).data());
        deviceCreateInfo.setPEnabledExtensionNames(device_extensions);

        m_device = physical_device().createDeviceUnique(deviceCreateInfo, nullptr, m_dispatch_loader_core);

        // --- G. Initialize Device Dispatchers ---
        if constexpr (std::is_same_v<std::remove_cv_t<decltype(m_dispatch_loader_core)>, vk::detail::DispatchLoaderDynamic>) {
            reinterpret_cast<vk::detail::DispatchLoaderDynamic*>(&m_dispatch_loader_core)->init(m_device.get());
        }
        if constexpr (std::is_same_v<std::remove_cv_t<decltype(m_dispatch_loader_ext)>, vk::detail::DispatchLoaderDynamic>) {
            reinterpret_cast<vk::detail::DispatchLoaderDynamic*>(&m_dispatch_loader_ext)->init(m_device.get());
        }

        // --- H. Finalize Queues & Pools ---
        queues[0].assign_handle();
        m_queues = std::move(queues[0]);

        VmaAllocatorCreateInfo allocator_info = {
            .physicalDevice = physical_device(),
            .device = device(),
            .instance = instance()
        };
        vmaCreateAllocator(&allocator_info, &m_memory_allocator);

        m_cmd_pool = create_command_pool(m_queues.family_index(), vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

        m_initialized = true;
        LOG_S(INFO) << "Vulkan Root successfully initialized.";
    }

    vk::Instance instance() {
        if (!m_instance)
        {
            initialize();
        }

        return m_instance.get();
    }

    vk::PhysicalDevice& physical_device() override {
        if (!m_physical_device)
        {
            initialize();
        }

        return m_physical_device;
    }
    const vk::PhysicalDevice& physical_device() const override {
        assert(m_physical_device);
        return m_physical_device;
    }

    vk::Device& device() override {
        if (!m_device)
        {
            initialize();
        }

        return m_device.get();
    }
    const vk::Device& device() const override {
        assert(m_device);
        return m_device.get();
    }

    DISPATCH_LOADER_CORE_TYPE& dispatch_loader_core() override {
        assert(m_dispatch_loader_core);
        return m_dispatch_loader_core;
    }
    const DISPATCH_LOADER_CORE_TYPE& dispatch_loader_core() const override {
        assert(m_dispatch_loader_core);
        return m_dispatch_loader_core;
    }

    DISPATCH_LOADER_EXT_TYPE& dispatch_loader_ext() override {
        assert(m_dispatch_loader_ext);
        return m_dispatch_loader_ext;
    }
    const DISPATCH_LOADER_EXT_TYPE& dispatch_loader_ext() const override {
        assert(m_dispatch_loader_ext);
        return m_dispatch_loader_ext;
    }

    const avk::queue& queue() {
        assert(m_queues);
        return m_queues;
    }

    avk::command_pool& command_pool() {
        assert(m_cmd_pool);
        return m_cmd_pool;
    }

    AVK_MEM_ALLOCATOR_TYPE& memory_allocator() override {
        assert(m_memory_allocator);
        return m_memory_allocator;
    }
    const AVK_MEM_ALLOCATOR_TYPE& memory_allocator() const override {
        assert(m_memory_allocator);
        return m_memory_allocator;
    }

    const avk::renderpass& default_renderpass();

private:
    bool m_initialized = false;

    vk::UniqueHandle<vk::Instance, DISPATCH_LOADER_CORE_TYPE> m_instance;
    vk::PhysicalDevice m_physical_device;
    vk::UniqueHandle<vk::Device, DISPATCH_LOADER_CORE_TYPE> m_device;

    DISPATCH_LOADER_CORE_TYPE m_dispatch_loader_core;
    DISPATCH_LOADER_EXT_TYPE m_dispatch_loader_ext;

    AVK_MEM_ALLOCATOR_TYPE m_memory_allocator;
    avk::queue m_queues;
    avk::command_pool m_cmd_pool;

    VkDebugUtilsMessengerEXT m_debug_messenger = VK_NULL_HANDLE;
    void create_debug_callback();
    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data, void* p_user_data);
    static const char* severity_str_mapper(VkDebugUtilsMessageSeverityFlagBitsEXT severity);
    static const char* type_str_mapper(VkDebugUtilsMessageTypeFlagsEXT type);
};