#include <native/graphics/shaders.obn.h>

auto ShaderBindings::to_vk_descriptor() const -> vk::DescriptorType
{
	switch (type)
	{
	case BindingType::UniformBuffer:
		return vk::DescriptorType::eUniformBuffer;
	case BindingType::StorageBuffer:
		return vk::DescriptorType::eStorageBuffer;
	case BindingType::CombinedImageSampler:
		return vk::DescriptorType::eCombinedImageSampler;
	case BindingType::SampledImage:
		return vk::DescriptorType::eSampledImage;
	case BindingType::StorageImage:
		return vk::DescriptorType::eStorageImage;
	case BindingType::AccelerationStructure:
		return vk::DescriptorType::eAccelerationStructureKHR;
	}

	throw std::runtime_error("Unsupported binding type in reflection!");
}

auto ShaderBindings::create_from_descriptor(const SpvReflectDescriptorBinding& descriptor, const vk::ShaderStageFlagBits stage) -> ShaderBindings
{
	ShaderBindings binding;
	binding.name = descriptor.name;
	binding.binding_index = descriptor.binding;
	binding.set_index = descriptor.set;
	binding.size = descriptor.block.size;
	binding.offset = descriptor.block.offset;
	binding.count = descriptor.count;
	binding.stages = stage;

	switch (descriptor.descriptor_type)
	{
	case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		binding.type = BindingType::UniformBuffer; break;
	case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		binding.type = BindingType::StorageBuffer; break;
	case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
		binding.type = BindingType::CombinedImageSampler; break;
	case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
		binding.type = BindingType::SampledImage; break;
	case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
		binding.type = BindingType::StorageImage; break;
	default:
		binding.type = BindingType::Unknown;
	}

	return binding;
}

auto PipelineBlueprint::add_shader(const ShaderType type, const vk::ShaderStageFlagBits stage, const avk::shader_info& shader_info, const std::vector<uint32_t>& spirv_binary) -> void
{
	SpvReflectShaderModule refl_module;
	if (spvReflectCreateShaderModule(spirv_binary.size() * sizeof(uint32_t), spirv_binary.data(), &refl_module) != SPV_REFLECT_RESULT_SUCCESS)
	{
		LOG_S(ERROR) << "Reflection failed for shader stage: " << vk::to_string(stage);
		return;
	}

	uint32_t count = 0;
	spvReflectEnumerateDescriptorBindings(&refl_module, &count, nullptr);
	std::vector<SpvReflectDescriptorBinding*> reflected_bindings(count);
	spvReflectEnumerateDescriptorBindings(&refl_module, &count, reflected_bindings.data());

	for (const auto* descriptor : reflected_bindings)
	{
		auto it = std::find_if(bindings.begin(), bindings.end(), [&](const ShaderBindings& binding) { 
			return binding.binding_index == descriptor->binding && binding.set_index == descriptor->set; 
		});

		if (it != bindings.end())
		{
			it->stages |= stage;
			continue;
		}

		ShaderBindings binding = ShaderBindings::create_from_descriptor(*descriptor, stage);
		bindings.push_back(binding);
		bindings_by_name.insert_or_assign(binding.name, binding);

		if (binding.type != ShaderBindings::BindingType::UniformBuffer)
		{
			continue;
		}

		for (std::size_t i = 0; i < descriptor->block.member_count; ++i)
		{
			const auto& member = descriptor->block.members[i];

			ShaderBindings member_binding = binding;
			member_binding.name = member.name;
			member_binding.size = member.padded_size;
			member_binding.offset = member.offset;
			member_binding.type = ShaderBindings::BindingType::UniformBufferMember;

			bindings.push_back(member_binding);
			bindings_by_name.insert_or_assign(member_binding.name, member_binding);
		}
	}

	uint32_t pc_count = 0;
	spvReflectEnumeratePushConstantBlocks(&refl_module, &pc_count, nullptr);
	std::vector<SpvReflectBlockVariable*> pcs(pc_count);
	spvReflectEnumeratePushConstantBlocks(&refl_module, &pc_count, pcs.data());

	if (pc_count > 0)
	{
		auto* pc = pcs[0];
		push_constant_size = std::max(push_constant_size, pc->size);
		push_constant_stages |= stage;
	}

	if (push_constant_size > 128)
	{
		LOG_S(WARNING) << "Shader " << shader_info.mPath << " requires " << push_constant_size << " bytes for its push constants. Vulkan can only guarantee 128!";
	}

	shaders[type].push_back(shader_info);
	spvReflectDestroyShaderModule(&refl_module);
}

auto PipelineBlueprint::has_shader(const ShaderType type) const -> bool
{
	return shaders.find(type) != shaders.end();
}

auto PipelineBlueprint::as_binding_data() const -> std::vector<avk::binding_data>
{
	std::vector<avk::binding_data> binding_data;

	for (const auto& binding : bindings)
	{
		if (binding.type == ShaderBindings::BindingType::UniformBufferMember)
		{
			continue;
		}

		avk::binding_data data;
		data.mSetId = binding.set_index;
		data.mLayoutBinding = vk::DescriptorSetLayoutBinding(
			binding.binding_index, binding.to_vk_descriptor(), binding.count, binding.stages, nullptr
		);

		binding_data.push_back(std::move(data));
	}

	return binding_data;
}

auto Pipeline::add_shader(ShaderType type, const avk::shader_info& shader_info) -> void
{
	m_shaders[type].push_back(shader_info);
	m_elements++;
}

auto Pipeline::set_type(PipelineType type) -> void
{
	m_type = type;
}

auto Pipeline::shaders(ShaderType type) const -> std::span<const avk::shader_info>
{
	auto it = m_shaders.find(type);
	if (it != m_shaders.end())
	{
		return it->second;
	}

	return {};
}

auto Pipeline::elements() const -> std::size_t
{
	return m_elements;
}

auto Pipeline::type() const -> PipelineType
{
	return m_type;
}

auto Pipeline::has_shader(ShaderType type) const -> bool
{
	return this->m_shaders.find(type) != this->m_shaders.end();
}

auto Pipeline::bind_into(avk::command_buffer_t& cmd_buffer) const -> void
{
	std::visit([&](auto& pipeline) {
		using T = std::decay_t<decltype(pipeline)>;
		if constexpr(IsValidPipeline<T>)
		{
			cmd_buffer.record(avk::command::bind_pipeline(pipeline.as_reference()));
		}
		else
		{
			LOG_S(ERROR) << "Attempted to bind an uninitialized pipeline!";
		}
	}, m_pipeline);
}

auto Pipeline::layout_handle() const -> vk::PipelineLayout
{
	return std::visit([&](auto& pipeline) -> vk::PipelineLayout {
		using T = std::decay_t<decltype(pipeline)>;
		if constexpr(IsValidPipeline<T>)
		{
			return pipeline->layout_handle();
		}
		else
		{
			LOG_S(ERROR) << "Attempted to get layout handle of an uninitialized pipeline!";
			return vk::PipelineLayout();
		}
	}, m_pipeline);
}

auto Pipeline::make_descriptor_set(avk::descriptor_pool& pool, uint32_t set_index) const -> vk::DescriptorSet
{
	return std::visit([&](auto& pipeline) -> vk::DescriptorSet {
		using T = std::decay_t<decltype(pipeline)>;
		if constexpr (IsValidPipeline<T>)
		{
			const auto& layout = pipeline->descriptor_set_layouts().set_at(set_index);
			return pool.allocate({ layout })[0];
		}
		else
		{
			LOG_S(ERROR) << "Attempted to create descriptor set from an uninitialized pipeline!";
			return vk::DescriptorSet();
		}
	}, m_pipeline);
}

auto ShaderManager::load(const std::string& shader_path, const std::string& compile_path) -> void
{
	struct ShaderLoadInfo
	{
		PipelineType pipeline_type;
		ShaderType shader_type;
		std::string name;
		std::function<avk::shader_info(const std::string&, std::string, bool)> loader;

		ShaderLoadInfo(PipelineType p, ShaderType t, const std::string& n, std::function<avk::shader_info(const std::string&, std::string, bool)> l)
			: pipeline_type(p), shader_type(t), name(n), loader(l) {
		}
	};

	static const std::map<std::string, ShaderLoadInfo> shader_mapper = {
		// Graphics shaders
		{ ".vert", { PipelineType::Graphics, ShaderType::Vertex, "vertex", avk::vertex_shader } },
		{ ".frag", { PipelineType::Graphics, ShaderType::Fragment, "fragment", avk::fragment_shader } },
		{ ".geom", { PipelineType::Graphics, ShaderType::Geometry, "geometry", avk::geometry_shader } },
		{ ".tesc", { PipelineType::Graphics, ShaderType::TessellationControl, "tesselation control", avk::tessellation_control_shader } },
		{ ".tese", { PipelineType::Graphics, ShaderType::TessellationEvaluation, "tesselation evaluation", avk::tessellation_evaluation_shader } },
		// Ray tracing shaders
		{ ".rgen", { PipelineType::RayTracing, ShaderType::RayGeneration, "ray generation", avk::ray_generation_shader } },
		{ ".rmiss", { PipelineType::RayTracing, ShaderType::Miss, "miss", avk::miss_shader } },
		{ ".rchit", { PipelineType::RayTracing, ShaderType::ClosestHit, "closest hit", avk::closest_hit_shader } },
		{ ".rahit", { PipelineType::RayTracing, ShaderType::AnyHit, "any hit", avk::any_hit_shader } },
		{ ".rint", { PipelineType::RayTracing, ShaderType::Intersection, "intersection", avk::intersection_shader } },
		// Compute shaders
		{ ".comp", { PipelineType::Compute, ShaderType::Compute, "compute", avk::compute_shader } },
	};

	for (const auto& entry : std::filesystem::directory_iterator(shader_path))
	{
		if (!entry.is_directory())
		{
			continue;
		}

		PipelineBlueprint blueprint;
		blueprint.name = entry.path().filename().string();

		LOG_S(INFO) << "Loading shader blueprint: " << blueprint.name;

		for (const auto& file : std::filesystem::directory_iterator(entry.path()))
		{
			if (!file.is_regular_file())
			{
				continue;
			}

			auto& file_path = file.path();
			auto file_ext = file_path.extension().string();
			auto file_name = file_path.filename().string();

			if (shader_mapper.find(file_ext) == shader_mapper.end())
			{
				LOG_S(WARNING) << "Skipping " << file_name << file_ext << " because it has an unsupported file extension.";
			}

			const ShaderLoadInfo& shader_info = shader_mapper.at(file_ext);
			if (blueprint.shaders.empty())	// Set pipeline type on first shader
			{
				blueprint.type = shader_info.pipeline_type;
			}

			if (shader_info.pipeline_type != blueprint.type) // Ensure shader matches pipeline type
			{
				LOG_S(WARNING) << "Skipping " << file_name << file_ext << " because it does not match the pipeline type.";
				continue;
			}

			if ((blueprint.type == PipelineType::Graphics || blueprint.type == PipelineType::Compute) && blueprint.has_shader(shader_info.shader_type)) // Ensure no duplicate shaders in graphics or compute pipeline blueprint
			{
				LOG_S(WARNING) << "Skipping " << file_name << file_ext << " because a " << shader_info.name << "shader has already been added to the graphics pipeline blueprint.";
				continue;
			}

			if (blueprint.type == PipelineType::RayTracing && shader_info.shader_type == ShaderType::RayGeneration && blueprint.has_shader(ShaderType::RayGeneration)) // Ensure only one ray generation shader in ray tracing pipeline blueprint
			{
				LOG_S(WARNING) << "Skipping " << file_name << file_ext << " because a ray generation shader has already been added to the ray tracing pipeline blueprint.";
				continue;
			}

			// Compile GLSL to SPIR-V
			// TODO: Figure out a way to store in cache (avoid writing to spv file)
			avk::shader_info glsl_info = shader_info.loader(file_path.string(), "main", false);
			auto shader_type = avk::to_vk_shader_stage(glsl_info.mShaderType);
			std::string glsl_code = ObsidianIO::read_file(file_path.string());

			std::vector<uint32_t> spirv_binary;
			bool result = SpirvTranslator::glsl_to_spv(shader_type, glsl_code.c_str(), spirv_binary);
			if (!result)
			{
				LOG_S(ERROR) << "Failed to compile GLSL shader " << file_name << " to SPIR-V.";
				continue;
			}

			std::filesystem::path spirv_folder_path = std::filesystem::path(compile_path) / blueprint.name;
			std::filesystem::create_directories(spirv_folder_path);
			std::filesystem::path spirv_path = spirv_folder_path / (file_path.filename().string() + ".spv");
			ObsidianIO::write_file(spirv_path.string(), spirv_binary);

			avk::shader_info spirv_info = shader_info.loader(spirv_path.string(), "main", false);
			blueprint.add_shader(shader_info.shader_type, shader_type, spirv_info, spirv_binary);
		}

		if (!blueprint.shaders.empty())
		{
			m_blueprints[blueprint.name] = std::move(blueprint);
		}
	}
}

auto ShaderManager::pipeline(const std::string& name, const Shard& shard, const PipelineOptions& options) -> Pipeline&
{
	return pipeline(name, shard.attachments, shard.depth_attachment, options);
}

auto ShaderManager::pipeline(const std::string& name, const std::vector<ShardMetadata>& color_attachments, const ShardMetadata& depth_attachment, const PipelineOptions& options) -> Pipeline&
{
	// Generate pipeline key
	std::vector<vk::Format>	color_formats;
	for (const auto& attachment : color_attachments)
	{
		if (attachment.usage.contains_color())
		{
			color_formats.push_back(attachment.format);
		}
	}

	vk::Format depth_format = vk::Format::eUndefined;
	if (depth_attachment.usage.contains_depth_stencil())
	{
		depth_format = depth_attachment.format;
	}

	const PipelineKey key{ name, color_formats, depth_format };

	// Return existing pipeline if found...
	if (m_pipelines.contains(key))
	{
		return m_pipelines[key];
	}

	// ...Or create new pipeline from blueprint
	if (!m_blueprints.contains(name))
	{
		throw std::runtime_error("Unable to create pipeline. No blueprint '" + name + "' found.");
	}

	const auto& blueprint = m_blueprints.at(name);

	LOG_S(INFO) << "Creating pipeline from blueprint: " << name;

	Pipeline pipeline;
	pipeline.set_type(blueprint.type);

	switch (pipeline.type())
	{
	case PipelineType::Graphics:
		build_graphics_pipeline(pipeline, blueprint, options, color_formats, depth_format);
		break;

	case PipelineType::RayTracing:
		build_raytracing_pipeline(pipeline, blueprint, options);
		break;

	case PipelineType::Compute:
		build_compute_pipeline(pipeline, blueprint);
		break;
	}

	m_pipelines[key] = std::move(pipeline);
	return m_pipelines[key];
}

auto ShaderManager::blueprint(const std::string& name) const -> const PipelineBlueprint&
{
	if (!m_blueprints.contains(name))
	{
		throw std::runtime_error("Blueprint '" + name + "' not found!");
	}

	return m_blueprints.at(name);
}

void ShaderManager::build_graphics_pipeline(Pipeline& pipeline, const PipelineBlueprint& blueprint, const PipelineOptions& options, const std::vector<vk::Format>& color_formats, const vk::Format depth_format)
{
	if (color_formats.empty())
	{
		throw std::runtime_error("No specified color formats!");
	}

	avk::graphics_pipeline_config config;
	const ShaderType shader_order[] = { ShaderType::Vertex, ShaderType::TessellationControl, ShaderType::TessellationEvaluation, ShaderType::Geometry, ShaderType::Fragment };
	for (const auto shader_type : shader_order)
	{
		if (blueprint.has_shader(shader_type))
		{
			config.mShaderInfos.push_back(
				blueprint.shaders.at(shader_type)[0]
			);
		}
	}

	if (blueprint.push_constant_size > 0)
	{
		config.mPushConstantsBindings.push_back(
			avk::push_constant_binding_data{ 
				static_cast<avk::shader_type>(static_cast<uint32_t>(blueprint.push_constant_stages)), 
				0, 
				blueprint.push_constant_size
			}
		);
	}
	
	config.mPrimitiveTopology =	options.topology;
	config.mPolygonDrawingModeAndConfig = avk::cfg::polygon_drawing{ options.polygon_mode, 1.0f, false, 1.0f };
	config.mCullingMode = options.cull_mode;
	config.mFrontFaceWindingOrder =	options.front_face;
	
	for (uint32_t i = 0; i < color_formats.size(); ++i)
	{
		auto blending_config = options.alpha_blending
			? avk::cfg::color_blending_config::enable_alpha_blending_for_attachment(i)
			: avk::cfg::color_blending_config::disable_blending_for_attachment(i);

		config.mColorBlendingPerAttachment.push_back(blending_config);
	}

	if (options.depth_test)
	{
		auto depth_test = avk::cfg::depth_test::enabled();
		depth_test.set_compare_operation(avk::cfg::compare_operation::greater_or_equal);
		config.mDepthTestConfig = depth_test;
	}
	else
	{
		config.mDepthTestConfig = avk::cfg::depth_test::disabled();
	}

	config.mDepthWriteConfig.mEnabled =	options.depth_write;
	config.mDynamicRendering = avk::cfg::dynamic_rendering::enabled;
	config.mViewportDepthConfig.push_back(avk::cfg::viewport_depth_scissors_config::dynamic(true, true));
	config.mResourceBindings = blueprint.as_binding_data();

	std::vector<avk::attachment> dyn_attachments;
	for (uint32_t i = 0; i < color_formats.size(); ++i)
	{
		dyn_attachments.push_back(
			avk::attachment::declare(
				color_formats[i],
				avk::on_load::clear,
				avk::usage::color(i),
				avk::on_store::store
			)
		);
	}

	if (depth_format != vk::Format::eUndefined)
	{
		dyn_attachments.push_back(
			avk::attachment::declare(
				depth_format,
				avk::on_load::clear,
				avk::usage::depth_stencil,
				avk::on_store::dont_care
			)
		);
	}

	config.mDynamicRenderingAttachments = std::move(dyn_attachments);

	config.mInputBindingLocations.push_back(avk::from_buffer_binding(0)->stream_per_vertex(&Vertex::pos)->to_location(0));
	config.mInputBindingLocations.push_back(avk::from_buffer_binding(0)->stream_per_vertex(&Vertex::normal)->to_location(1));
	config.mInputBindingLocations.push_back(avk::from_buffer_binding(0)->stream_per_vertex(&Vertex::uv)->to_location(2));

	pipeline.m_pipeline = this->m_root.create_graphics_pipeline(std::move(config)); // TODO IO-avoid: look at root::build_shader_module_from_binary_code...
}

void ShaderManager::build_raytracing_pipeline(Pipeline& pipeline, const PipelineBlueprint& blueprint, const PipelineOptions& options)
{
	// Get ray generation shader
	const auto& rgen = blueprint.shaders.at(ShaderType::RayGeneration)[0];

	// Get miss shaders
	std::vector<avk::shader_info> miss_shaders;
	if (blueprint.shaders.contains(ShaderType::Miss))
	{
		const auto& shaders = blueprint.shaders.at(ShaderType::Miss);
		miss_shaders.assign(shaders.begin(), shaders.end());
	}

	// Hit group magic
	std::vector<avk::triangles_hit_group> tri_hits;
	std::vector<avk::procedural_hit_group> proc_hits;
	std::set<std::string> processed_names;

	auto find_shader_by_name = [&](ShaderType type, const std::string& name) -> std::optional<avk::shader_info> {
		if (!blueprint.shaders.contains(type)) return std::nullopt;

		for (const auto& shader : blueprint.shaders.at(type)) {
			if (std::filesystem::path(shader.mPath).stem().string() == name) return shader;
		}

		return std::nullopt;
	};

	auto process_material = [&](const std::string& name) -> void {
		if (processed_names.contains(name)) return;

		auto chit = find_shader_by_name(ShaderType::ClosestHit, name);
		auto ahit = find_shader_by_name(ShaderType::AnyHit, name);
		auto rint = find_shader_by_name(ShaderType::Intersection, name);

		if (rint.has_value())
		{
			proc_hits.push_back(avk::procedural_hit_group{ rint.value(), ahit, chit});
		}
		else if (chit.has_value())
		{
			tri_hits.push_back(avk::triangles_hit_group{ ahit, chit });
		}

		processed_names.insert(name);
	};

	if (blueprint.shaders.contains(ShaderType::ClosestHit))
	{
		for (const auto& shader : blueprint.shaders.at(ShaderType::ClosestHit))
		{
			const std::string& name = std::filesystem::path(shader.mPath).stem().string();
			process_material(name);
		}
	}

	if (blueprint.shaders.contains(ShaderType::Intersection))
	{
		for (const auto& shader : blueprint.shaders.at(ShaderType::Intersection))
		{
			const std::string& name = std::filesystem::path(shader.mPath).stem().string();
			process_material(name);
		}
	}

	Variadic::apply_vector(miss_shaders, [&](auto&... expanded_miss) {
		Variadic::apply_vector(tri_hits, [&](auto&... expanded_tri_hits) {
			Variadic::apply_vector(proc_hits, [&](auto&... expanded_proc_hits) {
				auto master_sbt = avk::define_shader_table(
					std::move(rgen),
					std::move(expanded_miss)...,
					std::move(expanded_tri_hits)...,
					std::move(expanded_proc_hits)...
				);

				avk::ray_tracing_pipeline_config config;
				config.mMaxRecursionDepth.set_to(options.max_recursion_depth);
				config.mResourceBindings = blueprint.as_binding_data();
				config.mShaderTableConfig = std::move(master_sbt);

				if (blueprint.push_constant_size > 0)
				{
					config.mPushConstantsBindings.push_back(
						avk::push_constant_binding_data{
							static_cast<avk::shader_type>(static_cast<uint32_t>(blueprint.push_constant_stages)),
							0,
							blueprint.push_constant_size
						}
					);
				}

				pipeline.m_pipeline = m_root.create_ray_tracing_pipeline(std::move(config));
			});
		});
	});
}

void ShaderManager::build_compute_pipeline(Pipeline& pipeline, const PipelineBlueprint& blueprint)
{
	avk::compute_pipeline_config config;
	if (!blueprint.shaders.contains(ShaderType::Compute))
	{
		throw std::runtime_error("No compute shader found in compute pipeline blueprint!");
	}

	config.mShaderInfo = blueprint.shaders.at(ShaderType::Compute)[0];
	config.mResourceBindings = blueprint.as_binding_data();

	if (blueprint.push_constant_size > 0)
	{
		config.mPushConstantsBindings.push_back(
			{ avk::shader_type::compute, 0, blueprint.push_constant_size }
		);
	}

	pipeline.m_pipeline = this->m_root.create_compute_pipeline(std::move(config));
}