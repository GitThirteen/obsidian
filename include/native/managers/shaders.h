#pragma once

#include <native/core/root.h>
#include <native/util/variadic.h>

enum class ShaderType
{
	// Graphics shader types
	Vertex,
	Fragment,
	Geometry,
	TessellationControl,
	TessellationEvaluation,
	// Ray tracing shader types
	RayGeneration,
	Miss,
	ClosestHit,
	AnyHit,
	Intersection,
	// Compute shader
	Compute
};

enum class PipelineType
{
	Graphics,
	RayTracing,
	Compute
};

struct PipelineOptions
{
	avk::cfg::primitive_topology topology = avk::cfg::primitive_topology::triangles;
	avk::cfg::polygon_drawing_mode polygon_mode = avk::cfg::polygon_drawing_mode::fill;
	avk::cfg::culling_mode cull_mode = avk::cfg::culling_mode::cull_back_faces;
	avk::cfg::front_face front_face = avk::cfg::front_face::define_front_faces_to_be_counter_clockwise();

	bool depth_test = true;
	bool depth_write = true;

	bool alpha_blending = false;

	uint32_t max_recursion_depth = 1;

	static PipelineOptions create_default()
	{
		return PipelineOptions();
	}

	static PipelineOptions create_wireframe()
	{
		PipelineOptions options;
		options.polygon_mode = avk::cfg::polygon_drawing_mode::line;
		options.cull_mode = avk::cfg::culling_mode::disabled;
		return options;
	}

	static PipelineOptions create_transparent()
	{
		PipelineOptions options;
		options.depth_write = false;
		options.alpha_blending = true;
		return options;
	}
};

struct Pipeline
{
	using ShaderMap = std::unordered_map<ShaderType, std::vector<avk::shader_info>>;
	using AvkPipeline = std::variant<std::monostate, avk::graphics_pipeline, avk::ray_tracing_pipeline, avk::compute_pipeline>;

	Pipeline() = default;

	void add_shader(ShaderType type, const avk::shader_info& shader_info);
	void set_type(PipelineType type);
	std::span<const avk::shader_info> shaders(ShaderType type) const;
	std::size_t elements() const;
	PipelineType type() const;
	bool has_shader(ShaderType type) const;
	bool is_built() const;
	
private:
	friend class ShaderManager;

	bool m_built = false;
	PipelineType m_type;
	ShaderMap m_shaders;
	AvkPipeline m_pipeline;
	std::size_t m_elements = 0;
};

class ShaderManager
{
public:
	ShaderManager() = default;
	ShaderManager(Root& root) : m_root(root) { }

	void load(std::string path);
	Pipeline& get_pipeline(const std::string& name);
	void build_pipeline(const std::string& name, const PipelineOptions& options = PipelineOptions(), avk::renderpass renderpass = avk::renderpass(), uint32_t push_constant_size = 128);

private:
	Root& m_root;
	std::unordered_map<std::string, Pipeline> m_pipelines;

	void build_graphics_pipeline(Pipeline& pipeline, const PipelineOptions& options, avk::renderpass renderpass, uint32_t push_constant_size);
	void build_raytracing_pipeline(Pipeline& pipeline, const PipelineOptions& options, uint32_t push_constant_size);
	void build_compute_pipeline(Pipeline& pipeline, uint32_t push_constant_size);
};