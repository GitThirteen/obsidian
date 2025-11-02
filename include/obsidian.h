#pragma once

#include <./avk/avk.hpp>

#include <./native/_include.h>
#include <./native/_define.h>

class Obsidian : avk::root
{
public:
    Obsidian()
    {
        
    }

private:
    vk::Instance instance;
    vk::PhysicalDevice physical_device;
    vk::Device device;
    vk::DispatchLoaderDynamic dispatch_loader;
    std::array<avk::queue, 2> queues;
};