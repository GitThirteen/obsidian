#include <native/core/obsidian/obsidian.h>

void Obsidian::flow()
{
	int fps = Config::get<int>("fps");
	this->m_timer = FrameTimer(fps);
	vk::Extent2D extent = { Config::get<uint32_t>("dimensions", "width"), Config::get<uint32_t>("dimensions", "height") };

	// We'll do this here for now, expect this to be abstracted in the future
	this->scenes.create_scene("Testing",
		[](Scene& s, Root& root, WindowManager& window) {
			auto floor = Primitives::generate(root, GeometricPrimitiveType::Plane);
			floor->set_position({ 0.0f, -1.0f, 0.0f });
			floor->scale({ 10.0f, 1.0f, 10.0f });
			s.add_object(floor);

			auto cube = Primitives::generate(root, GeometricPrimitiveType::Cube);
			cube->set_position({ -1.5f, 0.0f, 0.0f });
			s.add_object(cube);

			auto sphere = Primitives::generate(root, GeometricPrimitiveType::Sphere);
			sphere->set_position({ 1.5f, 0.0f, 0.0f });
			s.add_object(sphere);

			auto camera = std::make_shared<UserCamera>(glm::vec3(0.0f, 1.0f, 5.0f), window.get_window());
			s.add_camera(camera);

			auto camera2 = std::make_shared<StaticCamera>(glm::vec3(0.0f, 1.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f));
			s.add_camera(camera2);
		},
		// We can add animations too :)
		[](Scene& s, double dt) {
			if (auto* cube = s.find_object("Cube1")) {
				cube->rotate(30.0f * (float)dt, { 0, 1, 0 });
			}
			if (auto* sphere = s.find_object("Sphere1")) {
				static float time = 0.0f;
				time += (float)dt;
				sphere->set_position({ 1.5f, sin(time) * 0.5f, 0.0f });
			}
		});

	scenes.switch_to("Testing");

    // We'll abstract this one too...
    std::vector<vk::DescriptorPoolSize> pools = {
        { vk::DescriptorType::eStorageBuffer, 1000 },
        { vk::DescriptorType::eUniformBuffer, 1000 },
        { vk::DescriptorType::eCombinedImageSampler, 1000 }
    };
    auto descriptor_pool = root.create_descriptor_pool(pools, 1000);

    while (!this->window.should_close())
    {
        this->m_timer.tick();
        this->events.poll();

        this->scenes.process_input();
        this->scenes.update(this->m_timer.dt());

        if (!this->frame.ready())
        { 
            continue;
        }

        auto& buffer = this->frame.current_command_buffer();
        auto& framebuffer = this->window.get_current_framebuffer(this->frame.current_image_index());
        auto& pipeline = this->shaders.get_pipeline("triangle").get<avk::graphics_pipeline>();

        auto& scene = this->scenes.active_scene();
        auto& cam = scene.active_camera();

        cam.set_aspect_ratio((float) extent.width, (float) extent.height);
        glm::mat4 viewProj = cam.get_view_projection();

        // We will abstract all of that away... Because jesus christ.
        this->root.record({
            avk::command::render_pass(
                root.default_renderpass().get(),
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
                        auto descriptor_set = descriptor_pool.allocate({
                            pipeline->descriptor_set_layouts().set_at(0)
                        });

                        vk::DescriptorSet raw_set = descriptor_set[0];
                        vk::DescriptorBufferInfo bufferInfo;
                        bufferInfo.setBuffer(obj->m_vertex_buffer.get().handle());
                        bufferInfo.setOffset(0);
                        bufferInfo.setRange(VK_WHOLE_SIZE);

                        vk::WriteDescriptorSet writeOp;
                        writeOp.setDstSet(raw_set);
                        writeOp.setDstBinding(0);
                        writeOp.setDstArrayElement(0);
                        writeOp.setDescriptorCount(1);
                        writeOp.setDescriptorType(vk::DescriptorType::eStorageBuffer);
                        writeOp.setPBufferInfo(&bufferInfo);

                        this->root.device().updateDescriptorSets(1, &writeOp, 0, nullptr);

                        cmd_buffer.handle().bindDescriptorSets(
                            vk::PipelineBindPoint::eGraphics,
                            pipeline->layout_handle(),
                            0,
                            1,
                            &raw_set,
                            0,
                            nullptr
                        );

                        struct PushData { glm::mat4 vp; glm::mat4 m; };
                        PushData pcData{ viewProj, obj->m_model_matrix };

                        cmd_buffer.handle().pushConstants(
                            pipeline.get().layout_handle(),
                            vk::ShaderStageFlagBits::eAll,
                            0,
                            sizeof(PushData),
                            &pcData
                        );

                        cmd_buffer.handle().bindIndexBuffer(obj->m_index_buffer.get().handle(), 0, vk::IndexType::eUint32);
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