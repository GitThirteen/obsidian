#include <native/core/obsidian.h>

auto Obsidian::flow() -> void
{
	int fps = Config::get<int>("fps");
	m_timer = FrameTimer(fps);

    while (!window.should_close())
    {
        m_timer.tick();
        events.poll();

        const auto renderpass = resources.renderpass();

        scenes.process_input();
        scenes.update(m_timer.dt());

        if (!frame.ready())
        { 
            continue;
        }

        auto& buffer = this->frame.current_command_buffer();
        auto& framebuffer = resources.framebuffer(frame.current_frame_index());
        auto& pipeline = this->shaders.get_pipeline("test").get<avk::graphics_pipeline>();

        auto& scene = this->scenes.active_scene();
        auto& camera = scene.active_camera();
        const auto extent = root.physical_device().getSurfaceCapabilitiesKHR(window.surface()).currentExtent;

        camera.set_aspect_ratio((float) extent.width, (float) extent.height);
        glm::mat4 viewproj = camera.get_view_projection();

        this->root.record({
            avk::command::render_pass(
                resources.renderpass().get(),
                framebuffer.get(),
                {
                    avk::command::bind_pipeline(pipeline.as_reference()),

                    avk::command::custom_commands([&](avk::command_buffer_t& cmd_buffer) {
                        vk::Viewport viewport{ 0.0f, 0.0f, (float)extent.width, (float)extent.height, 0.0f, 1.0f };
                        vk::Rect2D scissor{ {0, 0}, extent };
                        cmd_buffer.handle().setViewport(0, 1, &viewport);
                        cmd_buffer.handle().setScissor(0, 1, &scissor);
                    }),

                avk::command::custom_commands([&](avk::command_buffer_t& cmd_buffer) {
                    for (const auto& obj : scene.objects())
                    {
                        if (!obj->has_descriptor_set())
                        {
                            obj->create_descriptor_set(
                                resources.descriptor_pool(),
                                pipeline,
                                this->root
                            );
                        }

                        cmd_buffer.handle().bindDescriptorSets(
                            vk::PipelineBindPoint::eGraphics,
                            pipeline->layout_handle(),
                            0,
                            1,
                            &obj->descriptor_set(),
                            0,
                            nullptr
                        );

                        struct PushData { glm::mat4 vp; glm::mat4 m; };
                        PushData pcData{ viewproj, obj->model_matrix() };

                        cmd_buffer.handle().pushConstants(
                            pipeline->layout_handle(),
                            vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
                            0,
                            sizeof(PushData),
                            &pcData
                        );

                        cmd_buffer.handle().bindIndexBuffer(obj->index_buffer()->handle(), 0, vk::IndexType::eUint32);
                        cmd_buffer.handle().drawIndexed(obj->index_count(), 1, 0, 0, 0);
                    }
                })
            })
        }).into_command_buffer(buffer);

        this->frame.submit();
        this->m_timer.cap_fps();
    }

	this->root.device().waitIdle();
}

auto Obsidian::shatter() -> void
{
    resources.destroy();
    window.destroy();
}