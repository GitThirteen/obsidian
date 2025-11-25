#include <native/graphics/shaders.obn.h>

void Pipeline::add_shader(ShaderType type, const avk::shader_info& shader_info)
{
	this->m_shaders[type].push_back(shader_info);
	this->m_elements++;
}

void Pipeline::set_type(PipelineType type)
{
	this->m_type = type;
}

std::span<const avk::shader_info> Pipeline::shaders(ShaderType type) const
{
	auto it = this->m_shaders.find(type);
	if (it != this->m_shaders.end())
	{
		return it->second;
	}

	return {};
}

std::size_t Pipeline::elements() const
{
	return this->m_elements;
}

PipelineType Pipeline::type() const
{
	return this->m_type;
}

bool Pipeline::has_shader(ShaderType type) const
{
	return this->m_shaders.find(type) != this->m_shaders.end();
}

bool Pipeline::is_built() const
{
	return this->m_built;
}

void ShaderManager::load(const std::string& shader_path, const std::string& compile_path)
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

		Pipeline pipeline;
		auto pipeline_name = entry.path().filename().string();

		LOG_S(INFO) << "Loading shaders for pipeline: " << pipeline_name;
		
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
			if (pipeline.elements() == 0)	// Set pipeline type on first shader
			{
				pipeline.set_type(shader_info.pipeline_type);
			}

			if (shader_info.pipeline_type != pipeline.type()) // Ensure shader matches pipeline type
			{
				LOG_S(WARNING) << "Skipping " << file_name << file_ext << " because it does not match the pipeline type.";
				continue;
			}

			if ((pipeline.type() == PipelineType::Graphics || pipeline.type() == PipelineType::Compute) && pipeline.has_shader(shader_info.shader_type)) // Ensure no duplicate shaders in graphics or compute pipeline
			{
				LOG_S(WARNING) << "Skipping " << file_name << file_ext << " because a " << shader_info.name << "shader has already been added to the graphics pipeline.";
				continue;
			}

			if (pipeline.type() == PipelineType::RayTracing && shader_info.shader_type == ShaderType::RayGeneration && pipeline.has_shader(ShaderType::RayGeneration)) // Ensure only one ray generation shader in ray tracing pipeline
			{
				LOG_S(WARNING) << "Skipping " << file_name << file_ext << " because a ray generation shader has already been added to the ray tracing pipeline.";
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

			std::filesystem::path spirv_folder_path = std::filesystem::path(compile_path) / pipeline_name;
			std::filesystem::create_directories(spirv_folder_path);

			std::filesystem::path spirv_path = spirv_folder_path / (file_path.filename().string() + ".spv");
			ObsidianIO::write_file(spirv_path.string(), spirv_binary);

			avk::shader_info spirv_info = shader_info.loader(spirv_path.string(), "main", false);
			pipeline.add_shader(shader_info.shader_type, spirv_info);
		}

		if (pipeline.elements() > 0)
		{
			this->m_pipelines[pipeline_name] = pipeline;
		}
	}
}

Pipeline& ShaderManager::get_pipeline(const std::string& name)
{
	return this->m_pipelines.at(name);
}

void ShaderManager::build_pipeline(const std::string& name, avk::renderpass renderpass, const PipelineOptions& options, uint32_t push_constant_size)
{
	Pipeline& pipeline = get_pipeline(name);
	if (pipeline.is_built()) return;

	switch (pipeline.type())
	{
		case PipelineType::Graphics:
		{
			if (!pipeline.has_shader(ShaderType::Vertex))
			{
				throw std::runtime_error("Unable to build graphics pipeline for '" + name + "'. No vertex shader found.");
			}

			build_graphics_pipeline(pipeline, options, renderpass, push_constant_size);
			break;
		}

		case PipelineType::RayTracing:
		{
			if (!pipeline.has_shader(ShaderType::RayGeneration))
			{
				throw std::runtime_error("Unable to build ray-tracing pipeline for '" + name + "'. No ray generation shader found.");
			}

			build_raytracing_pipeline(pipeline, options, push_constant_size);
			break;
		}

		case PipelineType::Compute:
		{
			if (!pipeline.has_shader(ShaderType::Compute)) {
				throw std::runtime_error("Unable to build compute pipeline for '" + name + "'. No compute shader found.");
			}

			build_compute_pipeline(pipeline, push_constant_size);
			break;
		}

		default: return;
	}

	pipeline.m_built = true;
}

void ShaderManager::build_graphics_pipeline(Pipeline& pipeline, const PipelineOptions& options, avk::renderpass renderpass, uint32_t push_constant_size)
{
	if (!renderpass.has_value())
	{
		throw std::runtime_error("Invalid renderpass!");
	}

	avk::graphics_pipeline_config config;
	std::vector<ShaderType> shader_order = { ShaderType::Vertex, ShaderType::TessellationControl, ShaderType::TessellationEvaluation, ShaderType::Geometry, ShaderType::Fragment };

	for (const auto shader_type : shader_order)
	{
		if (pipeline.has_shader(shader_type))
		{
			config.mShaderInfos.push_back(pipeline.shaders(shader_type)[0]);
		}
	}

	config.mRenderPassSubpass = std::make_tuple(renderpass, 0);

	auto avk_shader_type = pipeline.has_shader(ShaderType::Fragment) ? avk::shader_type::all : avk::shader_type::vertex;

	config.mPushConstantsBindings.push_back(avk::push_constant_binding_data{ avk_shader_type, 0, push_constant_size });
	config.mPrimitiveTopology = options.topology;
	config.mPolygonDrawingModeAndConfig = avk::cfg::polygon_drawing{ options.polygon_mode, 1.0f, false, 1.0f };
	config.mCullingMode = options.cull_mode;
	config.mFrontFaceWindingOrder = options.front_face;
	
	auto blending_config = options.alpha_blending
		? avk::cfg::color_blending_config::enable_alpha_blending_for_attachment(0)
		: avk::cfg::color_blending_config::disable_blending_for_attachment(0);

	auto depth_test_config = options.depth_test
		? avk::cfg::depth_test::enabled()
		: avk::cfg::depth_test::disabled();

	auto depth_write_config = options.depth_write
		? avk::cfg::depth_write::enabled()
		: avk::cfg::depth_write::disabled();

	config.mColorBlendingPerAttachment.push_back(blending_config);
	config.mDepthTestConfig = depth_test_config;
	config.mDepthWriteConfig = depth_write_config;

	config.mDynamicRendering = avk::cfg::dynamic_rendering::disabled();
	config.mViewportDepthConfig.push_back(avk::cfg::viewport_depth_scissors_config::dynamic(true, true));

	avk::binding_data vbb;
	vbb.mSetId = 0;
	vbb.mLayoutBinding = vk::DescriptorSetLayoutBinding(
		0,
		vk::DescriptorType::eStorageBuffer,
		1,
		vk::ShaderStageFlagBits::eVertex,
		nullptr
	);

	config.mResourceBindings.push_back(std::move(vbb));

	pipeline.m_pipeline = this->m_root.create_graphics_pipeline(std::move(config));
}

void ShaderManager::build_raytracing_pipeline(Pipeline& pipeline, const PipelineOptions& options, uint32_t push_constant_size)
{
	// Get ray generation shader
	auto rgen = pipeline.shaders(ShaderType::RayGeneration)[0];

	// Get miss shaders
	std::vector<avk::shader_info> miss_shaders;
	if (pipeline.has_shader(ShaderType::Miss))
	{
		const auto& shaders = pipeline.shaders(ShaderType::Miss);
		miss_shaders.assign(shaders.begin(), shaders.end());
	}

	// Hit group magic
	std::vector<avk::triangles_hit_group> tri_hits;
	std::vector<avk::procedural_hit_group> proc_hits;
	std::set<std::string> processed_names;

	auto find_shader_by_name = [&](ShaderType type, const std::string& name) -> std::optional<avk::shader_info> {
		if (!pipeline.has_shader(type)) return std::nullopt;

		for (const auto& shader : pipeline.shaders(type)) {
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

	if (pipeline.has_shader(ShaderType::ClosestHit))
	{
		for (const auto& shader : pipeline.shaders(ShaderType::ClosestHit))
		{
			const std::string& name = std::filesystem::path(shader.mPath).stem().string();
			process_material(name);
		}
	}

	if (pipeline.has_shader(ShaderType::Intersection))
	{
		for (const auto& shader : pipeline.shaders(ShaderType::Intersection))
		{
			const std::string& name = std::filesystem::path(shader.mPath).stem().string();
			process_material(name);
		}
	}

	auto config = avk::push_constant_binding_data{ avk::shader_type::all, 0, push_constant_size };

	Variadic::apply_vector(miss_shaders, [&](auto&... expanded_miss) {
		Variadic::apply_vector(tri_hits, [&](auto&... expanded_tri_hits) {
			Variadic::apply_vector(proc_hits, [&](auto&... expanded_proc_hits) {
				auto master_sbt = avk::define_shader_table(
					std::move(rgen),
					std::move(expanded_miss)...,
					std::move(expanded_tri_hits)...,
					std::move(expanded_proc_hits)...
				);

				pipeline.m_pipeline = this->m_root.create_ray_tracing_pipeline_for(
					std::move(master_sbt),
					config,
					avk::max_recursion_depth(options.max_recursion_depth)
				);
			});
		});
	});
}

void ShaderManager::build_compute_pipeline(Pipeline& pipeline, uint32_t push_constant_size)
{
	auto& comp_shader = pipeline.shaders(ShaderType::Compute)[0];

	auto config = avk::push_constant_binding_data{ avk::shader_type::compute, 0, push_constant_size };

	pipeline.m_pipeline = this->m_root.create_compute_pipeline_for(
		comp_shader,
		config
	);
}