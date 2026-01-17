#include <native/graphics/envmap.h>

auto EnvironmentMap::load(const std::string& path) -> EnvironmentMap
{
    EnvironmentMap envmap;
    auto& dims = envmap.dimensions;

    float* data = stbi_loadf(path.c_str(), &dims.x(), &dims.y(), nullptr, 4);
    if (!data)
    {
        throw std::runtime_error("Failed to load HDR image: " + path);
    }

    envmap.channels = 4;
    envmap.pixels.assign(data, data + (dims.x() * dims.y() * envmap.channels));
    stbi_image_free(data);

    return envmap;
}

auto EnvironmentMap::upload(Root& root) -> void
{
    if (pixels.empty())
    {
        throw std::runtime_error("Attempted to upload empty environment map to GPU!");
    }

    auto staging = root.create_buffer(avk::memory_usage::host_visible, vk::BufferUsageFlagBits::eTransferSrc, avk::generic_buffer_meta::create_from_data(pixels));
    staging->fill(pixels.data(), 0);

    image = root.create_image(dimensions.x(), dimensions.y(), vk::Format::eR32G32B32A32Sfloat, 1, avk::memory_usage::device, avk::image_usage::sampled | avk::image_usage::transfer_destination);

    auto cmd = root.command_pool()->alloc_command_buffer(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    cmd->begin_recording();

    ImageBarrier::transition(cmd->handle(), image->handle())
        .from(vk::ImageLayout::eUndefined)
        .to(vk::ImageLayout::eTransferDstOptimal);

    vk::BufferImageCopy copy_region{};
    copy_region.imageSubresource = { vk::ImageAspectFlagBits::eColor, 0, 0, 1 };
    copy_region.imageExtent = vk::Extent3D{ (uint32_t) dimensions.x(), (uint32_t) dimensions.y(), 1};

    cmd->handle().copyBufferToImage(staging->handle(), this->image->handle(), vk::ImageLayout::eTransferDstOptimal, 1, &copy_region);

    ImageBarrier::transition(cmd->handle(), image->handle())
        .from(vk::ImageLayout::eTransferDstOptimal)
        .to(vk::ImageLayout::eShaderReadOnlyOptimal);

    cmd->end_recording();

    auto fence = root.device().createFenceUnique({});
    vk::SubmitInfo submit{};
    submit.setCommandBufferCount(1);
    submit.setPCommandBuffers(&cmd->handle());

    root.queue().handle().submit(1, &submit, fence.get());
    root.device().waitForFences(1, &fence.get(), VK_TRUE, UINT64_MAX);

    this->view = root.create_image_view(this->image);
    this->sampler = root.create_sampler(avk::filter_mode::bilinear, avk::border_handling_mode::clamp_to_edge);
}