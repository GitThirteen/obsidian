#include <native/core/obsidian.h>

auto Obsidian::flow() -> void
{
    const auto& shard_order = Config::get<std::vector<std::string>>("order");
    const int fps = Config::get<int>("fps");

	m_timer = FrameTimer(fps);
    window.ui.initialize(root, window.glfw_window(), resources);

    const auto& frames = resources.frames_in_flight();
    m_camera_buffers.resize(frames);
    m_light_buffers.resize(frames);
    m_global_descriptor_sets.resize(frames);

    // Initialize camera & light buffers per in-flight frame
    for (size_t i = 0; i < frames; ++i)
    {
        m_camera_buffers[i] = root.create_buffer(avk::memory_usage::host_visible, vk::BufferUsageFlagBits::eUniformBuffer, avk::generic_buffer_meta::create_from_data(CameraData{}));
        m_light_buffers[i] = root.create_buffer(avk::memory_usage::host_visible, vk::BufferUsageFlagBits::eUniformBuffer, avk::generic_buffer_meta::create_from_data(GPULightBlock{}));
    }

    // Particle System stuff
    // TODO: This should be its own class
    struct GPUParticle { glm::vec4 pos; glm::vec4 vel; };
    std::vector<GPUParticle> initial_particles(OBSIDIAN_NUM_PARTICLES);

    m_particle_buffer = root.create_buffer(avk::memory_usage::device, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, avk::storage_buffer_meta::create_from_data(initial_particles));
    m_particle_buffer->fill(initial_particles.data(), 0);

    auto& main_shard = resources.shards().at("main");
    auto& rain_draw_pipeline = shaders.pipeline("rain_draw", main_shard);
    auto& rain_sim_pipeline = shaders.pipeline("rain_sim", {}, {}, PipelineOptions::Type::Default);

    m_rain_sim_set = rain_sim_pipeline.make_descriptor_set(resources.descriptor_pool(), 1);
    ShaderDescriptor::write_storage_buffer(root.device(), m_rain_sim_set, 0, m_particle_buffer);

    m_rain_draw_set = rain_draw_pipeline.make_descriptor_set(resources.descriptor_pool(), 1);
    ShaderDescriptor::write_storage_buffer(root.device(), m_rain_draw_set, 0, m_particle_buffer);

    // Prewarming
    Pipeline* base_pipeline = nullptr;

    for (const auto& shard_name : shard_order)
    {
        auto& shard = resources.shards().at(shard_name);

        for (const auto& pipeline_name : shard.pipelines)
        {
            auto& pipeline = shaders.pipeline(pipeline_name, shard);

            if (!base_pipeline)
            {
                base_pipeline = &pipeline;
            }
        }
    }

    for (size_t i = 0; i < frames; ++i)
    {
        m_global_descriptor_sets[i] = base_pipeline->make_descriptor_set(resources.descriptor_pool(), 0);

        ShaderDescriptor::write_uniform_buffer(root.device(), m_global_descriptor_sets[i], 0, m_camera_buffers[i], 0, VK_WHOLE_SIZE);
        ShaderDescriptor::write_uniform_buffer(root.device(), m_global_descriptor_sets[i], 1, m_light_buffers[i], 0, VK_WHOLE_SIZE);
    }
    
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

        camera.set_aspect_ratio(static_cast<float>(extent.width), static_cast<float>(extent.height)); // TODO: Setting this separately is superfluous if the cam pos already contains it

        CameraData cam_data;
        cam_data.view_proj = camera.get_view_projection();
        cam_data.pos = glm::vec4(camera.position(), static_cast<float>(extent.width) / static_cast<float>(extent.height));

        GPULightBlock light_data;
        light_data.ambient = scene.ambient_light();
        light_data.directional = scene.directional_light();
        for (size_t i = 0; i < scene.lights().size(); ++i)
        {
            light_data.scene_lights[i] = scene.lights()[i];
        }

        auto in_flight_index = frame.current_frame_index();
        m_camera_buffers[in_flight_index]->fill(&cam_data, 0);
        m_light_buffers[in_flight_index]->fill(&light_data, 0);

        auto& envmap = scene.envmap();
        envmap.upload(root);
        ShaderDescriptor::write_combined_image_sampler(root.device(), m_global_descriptor_sets[in_flight_index], 2, envmap.view, envmap.sampler, vk::ImageLayout::eShaderReadOnlyOptimal);

        auto& cb = frame.current_command_buffer();
        auto cl = command_list(scene, shard_order, m_global_descriptor_sets[in_flight_index]);
        root.record(cl).into_command_buffer(cb);

        this->frame.submit();
        this->m_timer.cap_fps();
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

auto Obsidian::command_list(Scene& scene, const std::vector<std::string>& shard_order, vk::DescriptorSet global_set) -> std::vector<avk::recorded_commands_t>
{
    return {
        avk::command::custom_commands([&](avk::command_buffer_t& command_buffer) {
            // Particle System
            auto& rain_sim_pipeline = shaders.pipeline("rain_sim", {}, {}, PipelineOptions::Type::Default);
            rain_sim_pipeline.bind_into(command_buffer);
            command_buffer.handle().bindDescriptorSets(vk::PipelineBindPoint::eCompute, rain_sim_pipeline.layout_handle(), 1, 1, &m_rain_sim_set, 0, nullptr);

            struct ParticlePC { float dt; PADDING(12); glm::vec4 min; glm::vec4 max; glm::vec4 wind; };
            ParticlePC ppc;
            ppc.dt = m_timer.dt();
            glm::vec3 cam_pos = scene.active_camera().position();
            float rain_bounds = 80.0f; // TODO: Only spawn rain in octree nodes that are visible
            ppc.min = glm::vec4(cam_pos.x - rain_bounds,           - 10.0f, cam_pos.z - rain_bounds, 1.0f);
            ppc.max = glm::vec4(cam_pos.x + rain_bounds, cam_pos.y + 30.0f, cam_pos.z + rain_bounds, 1.0f);
            ppc.wind = glm::vec4(-2.0f, 0.0f, 0.0f, 0.0f);

            const auto& sim_blueprint = shaders.blueprint("rain_sim");
            command_buffer.handle().pushConstants(rain_sim_pipeline.layout_handle(), sim_blueprint.push_constant_stages, 0, sizeof(ppc), &ppc);
            command_buffer.handle().dispatch((OBSIDIAN_NUM_PARTICLES + 255) / 256, 1, 1);

            // TODO: Abstract Buffer memory barrier into barrier handler like image barrier
            vk::BufferMemoryBarrier barrier;
            barrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
            barrier.buffer = m_particle_buffer->handle();
            barrier.offset = 0;
            barrier.size = VK_WHOLE_SIZE;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

            command_buffer.handle().pipelineBarrier(
                vk::PipelineStageFlagBits::eComputeShader,
                vk::PipelineStageFlagBits::eVertexShader,
                vk::DependencyFlags(), 0, nullptr, 1, &barrier, 0, nullptr
            );

            // Rest
            uint32_t image_index = frame.current_image_index();
            auto swapchain_image = resources.swapchain_image(image_index)->handle();
            auto& envmap = scene.envmap();

            ImageBarrier::transition(command_buffer.handle(), swapchain_image)
                .from(vk::ImageLayout::eUndefined).as(vk::AccessFlagBits::eNone, vk::PipelineStageFlagBits::eTopOfPipe)
                .to(vk::ImageLayout::eColorAttachmentOptimal).as(vk::AccessFlagBits::eColorAttachmentWrite, vk::PipelineStageFlagBits::eColorAttachmentOutput)
                .commit();

            ImageBarrier::transition(command_buffer.handle(), envmap.view->get_image().handle())
                .from(vk::ImageLayout::eTransferDstOptimal).as(vk::AccessFlagBits::eTransferWrite, vk::PipelineStageFlagBits::eTransfer)
                .to(vk::ImageLayout::eShaderReadOnlyOptimal).as(vk::AccessFlagBits::eShaderRead, vk::PipelineStageFlagBits::eFragmentShader)
                .commit();

            auto extent = window.extent();
            vk::Viewport viewport{ 0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.0f, 1.0f };
            vk::Rect2D scissor({ 0, 0 }, extent);

            for (const auto& shard_name : shard_order)
            {
                const auto& shard = resources.shards().at(shard_name);

                std::vector<vk::RenderingAttachmentInfo> color_attachments;
                vk::RenderingAttachmentInfo depth_attachment;
                bool has_depth = false;

                if (shard_name == "postprocess")
                {
                    ImageBarrier::transition(command_buffer.handle(), swapchain_image)
                        .from(vk::ImageLayout::eColorAttachmentOptimal).as(vk::AccessFlagBits::eColorAttachmentWrite, vk::PipelineStageFlagBits::eColorAttachmentOutput)
                        .to(vk::ImageLayout::eColorAttachmentOptimal).as(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite, vk::PipelineStageFlagBits::eColorAttachmentOutput)
                        .commit();
                }

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

                    if (info.loadOp == vk::AttachmentLoadOp::eClear)
                    {
                        auto clear_color = (shard.is_swapchain_target && i == 0)
                            ? window.ui.clear_color()
                            : attachment.clear_color;
                        
                        info.clearValue = vk::ClearValue{ clear_color };
                    }
                    
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
                    
                    if (depth_attachment.loadOp == vk::AttachmentLoadOp::eClear)
                    {
                        depth_attachment.clearValue = vk::ClearValue{ { 0.0f, 0 } };
                    }
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

                // TODO: Compute shaders don't live in shards currently -- add that (or find another solution) so this becomes less messy
                for (const auto& pipeline_name : shard.pipelines)
                {
                    auto& pipeline = shaders.pipeline(pipeline_name, shard);
                    pipeline.bind_into(command_buffer);

                    command_buffer.handle().setViewport(0, 1, &viewport);
                    command_buffer.handle().setScissor(0, 1, &scissor);
                    command_buffer.handle().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.layout_handle(), 0, 1, &global_set, 0, nullptr);

                    if (pipeline_name == "skybox" || pipeline_name == "lensflare")
                    {
                        command_buffer.handle().draw(3, 1, 0, 0);
                        continue;
                    }

                    if (pipeline_name == "rain_draw")
                    {
                        command_buffer.handle().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.layout_handle(), 1, 1, &m_rain_draw_set, 0, nullptr);
                        command_buffer.handle().draw(OBSIDIAN_NUM_PARTICLES * 6, 1, 0, 0);
                        continue;
                    }

                    // TODO: batch drawing calls!!!
                    const auto& blueprint = shaders.blueprint(pipeline_name);
                    for (const auto& object : scene.objects())
                    {
                        if (std::find(object->pipelines().begin(), object->pipelines().end(), pipeline_name) == object->pipelines().end())
                        {
                            continue;
                        }

                        if (!object->has_descriptor_set())
                        {
                            object->create_descriptor_set(root, resources.descriptor_pool(), pipeline);
                        }

                        glm::mat4 inv_model = glm::inverse(object->model_matrix());
                        glm::vec4 local_cam_pos = inv_model * glm::vec4(scene.active_camera().position(), 1.0f);
                        local_cam_pos += 0.5f;

                        struct PushData { glm::mat4 model; glm::vec4 local_cam_pos; };
                        PushData pc_data = { object->model_matrix(), local_cam_pos };
                        command_buffer.handle().pushConstants(pipeline.layout_handle(), blueprint.push_constant_stages, 0, sizeof(PushData), &pc_data);
                        command_buffer.handle().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.layout_handle(), 1, 1, &object->descriptor_set(), 0, nullptr);

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

            window.ui.render(command_buffer.handle(), resources.swapchain_view(image_index)->handle());

            ImageBarrier::transition(command_buffer.handle(), swapchain_image)
                .from(vk::ImageLayout::eColorAttachmentOptimal).as(vk::AccessFlagBits::eColorAttachmentWrite, vk::PipelineStageFlagBits::eColorAttachmentOutput)
                .to(vk::ImageLayout::ePresentSrcKHR).as(vk::AccessFlagBits::eNone, vk::PipelineStageFlagBits::eBottomOfPipe)
                .commit();
        })
    };
}