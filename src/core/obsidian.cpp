#include <native/core/obsidian.h>

auto Obsidian::flow() -> void
{
    const auto& shard_order = Config::get<std::vector<std::string>>("order");
    const int fps = Config::get<int>("fps");
	m_timer = FrameTimer(fps);

    m_camera_buffer = root.create_buffer(avk::memory_usage::host_visible, vk::BufferUsageFlagBits::eUniformBuffer, avk::generic_buffer_meta::create_from_data(CameraData{}));

    while (!window.should_close())
    {
        m_timer.tick();
        events.poll();

        scenes.process_input();
        scenes.update(m_timer.dt());

        if (!frame.ready())
        {
            continue;
        }

        auto& scene = scenes.active_scene();
        auto& camera = scene.active_camera();
        const auto& extent = window.extent();

        camera.set_aspect_ratio(static_cast<float>(extent.width), static_cast<float>(extent.height));

        auto& cb = frame.current_command_buffer();
        auto cl = command_list(scene, shard_order);
        // TODO: optimize
        // TODO: pre-warm pipelines once before rendering starts, don't create them on the fly here
        root.record(cl).into_command_buffer(cb);

        this->frame.submit();
        this->m_timer.cap_fps(); // TODO: fix accuracy
    }

	this->root.device().waitIdle();
}

auto Obsidian::shatter() -> void
{
    LOG_S(INFO) << "Shattering Obsidian...";

    if (root.device())
    {
        root.device().waitIdle();
    }

    // TODO: Destroy scenes?
    // TODO: Destroy shaders?
    resources.destroy();
    window.destroy();
    root.destroy();
}

auto Obsidian::command_list(Scene& scene, const std::vector<std::string>& shard_order) -> std::vector<avk::recorded_commands_t>
{
    return {
        avk::command::custom_commands([&](avk::command_buffer_t& command_buffer) {
            uint32_t image_index = frame.current_image_index();

            auto extent = window.extent();
            vk::Viewport viewport{ 0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.0f, 1.0f };
            vk::Rect2D scissor({ 0, 0 }, extent);

            command_buffer.handle().setViewport(0, 1, &viewport);
            command_buffer.handle().setScissor(0, 1, &scissor);

            {
                vk::ImageMemoryBarrier barrier{};
                barrier.oldLayout = vk::ImageLayout::eUndefined;
                barrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.image = resources.swapchain_image(image_index)->handle();
                barrier.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };
                barrier.srcAccessMask = vk::AccessFlagBits::eNone;
                barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

                command_buffer.handle().pipelineBarrier(
                    vk::PipelineStageFlagBits::eTopOfPipe,
                    vk::PipelineStageFlagBits::eColorAttachmentOutput,
                    vk::DependencyFlags{},
                    0, nullptr,
                    0, nullptr,
                    1, &barrier
                );
            }

            for (const auto& shard_name : shard_order)
            {
                const auto& shard = resources.shards().at(shard_name);

                std::vector<vk::RenderingAttachmentInfo> color_attachments;
                vk::RenderingAttachmentInfo depth_attachment;
                bool has_depth = false;

                for (size_t i = 0; i < shard.attachments.size(); ++i)
                {
                    const auto& attachment = shard.attachments[i];
                    if (attachment.format == vk::Format::eUndefined)
                    {
                        continue;
                    }

                    vk::RenderingAttachmentInfo info;
                    info.imageView = (shard.is_swapchain_target)
                        ? resources.swapchain_view(image_index)->handle()
                        : shard.internal_views[i]->handle();
                    info.imageLayout = attachment.layout.mLayout;
                    info.loadOp = Translator::to<vk::AttachmentLoadOp>(attachment.load_op.mLoadBehavior);
                    info.storeOp = Translator::to<vk::AttachmentStoreOp>(attachment.store_op.mStoreBehavior);
                    info.clearValue = vk::ClearValue{ attachment.clear_color };

                    color_attachments.push_back(info);
                }

                if (shard.depth_attachment.format != vk::Format::eUndefined)
                {
                    has_depth = true;
                    const auto& d_att = shard.depth_attachment;
                    depth_attachment.imageView = (shard.is_swapchain_target)
                        ? resources.depth_view()->handle()
                        : shard.internal_depth_view->handle();
                    depth_attachment.imageLayout = d_att.layout.mLayout;
                    depth_attachment.loadOp = Translator::to<vk::AttachmentLoadOp>(d_att.load_op.mLoadBehavior);
                    depth_attachment.storeOp = Translator::to<vk::AttachmentStoreOp>(d_att.store_op.mStoreBehavior);
                    depth_attachment.clearValue = vk::ClearValue{ { 0.0f, 0 } };
                }

                vk::RenderingInfo rendering_info;
                rendering_info.renderArea = vk::Rect2D{ { 0, 0 }, window.extent() };
                rendering_info.layerCount = 1;
                rendering_info.colorAttachmentCount = static_cast<uint32_t>(color_attachments.size());
                rendering_info.pColorAttachments = color_attachments.data();

                if (has_depth)
                {
                    rendering_info.pDepthAttachment = &depth_attachment;
                }

                command_buffer.handle().beginRendering(rendering_info);

                for (const auto& pipeline_name : shard.pipelines)
                {
                    const auto& blueprint = shaders.blueprint(pipeline_name);
                    auto& pipeline = shaders.pipeline(pipeline_name, shard);

                    switch (pipeline.type())
                    {
                    case PipelineType::Graphics:
                        command_buffer.record(avk::command::bind_pipeline(pipeline.get<avk::graphics_pipeline>().as_reference()));
                        break;
                    case PipelineType::Compute:
                        command_buffer.record(avk::command::bind_pipeline(pipeline.get<avk::compute_pipeline>().as_reference()));
                        break;
                    case PipelineType::RayTracing:
                        command_buffer.record(avk::command::bind_pipeline(pipeline.get<avk::ray_tracing_pipeline>().as_reference()));
                        break;
                    default:
                        throw std::runtime_error("How did you get here?");
                    }

                    CameraData currentData;
                    currentData.view_proj = scene.active_camera().get_view_projection();
                    currentData.pos = glm::vec4(scene.active_camera().position(), 1.0f);

                    m_camera_buffer->fill(&currentData, 0);

                    // TODO: batch drawing calls!!!
                    for (const auto& object : scene.objects())
                    {
                        if (std::find(object->pipelines().begin(), object->pipelines().end(), pipeline_name) == object->pipelines().end())
                        {
                            continue;
                        }

                        if (!object->has_descriptor_set()) {
                            object->create_descriptor_set(root, resources.descriptor_pool(), pipeline, m_camera_buffer);
                        }

                        glm::mat4 inv_model = glm::inverse(object->model_matrix());
                        glm::vec4 local_cam_pos = inv_model * glm::vec4(scene.active_camera().position(), 1.0f);
                        local_cam_pos += 0.5f;

                        struct PushData { glm::mat4 model; glm::vec4 local_cam_pos; };
                        PushData pc_data = { object->model_matrix(), local_cam_pos };
                        command_buffer.handle().pushConstants(pipeline.layout_handle(), blueprint.push_constant_stages, 0, sizeof(PushData), &pc_data);

                        command_buffer.handle().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.layout_handle(), 0, 1, &object->descriptor_set(), 0, nullptr);

                        if (object->index_count() > 0)
                        {
                            vk::DeviceSize offset = 0;
                            vk::Buffer v_buffer = object->vertex_buffer()->handle();

                            command_buffer.handle().bindVertexBuffers(0, 1, &v_buffer, &offset);
                            command_buffer.handle().bindIndexBuffer(object->index_buffer()->handle(), 0, vk::IndexType::eUint32);
                            command_buffer.handle().drawIndexed(static_cast<uint32_t>(object->index_count()), 1, 0, 0, 0);
                        }
                    }
                }

                command_buffer.handle().endRendering();
            }

            {
                vk::ImageMemoryBarrier barrier{};
                barrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
                barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.image = resources.swapchain_image(image_index)->handle();
                barrier.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };
                barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
                barrier.dstAccessMask = vk::AccessFlagBits::eNone;

                command_buffer.handle().pipelineBarrier(
                    vk::PipelineStageFlagBits::eColorAttachmentOutput,
                    vk::PipelineStageFlagBits::eBottomOfPipe,
                    vk::DependencyFlags{},
                    0, nullptr,
                    0, nullptr,
                    1, & barrier
                );
            }
        })
    };
}