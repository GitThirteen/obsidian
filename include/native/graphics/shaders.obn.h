#pragma once

#include <native/core/include.h>
#include <native/core/root.h>
#include <native/graphics/compiler.h>
#include <native/utils/templates.h>
#include <native/utils/io.h>

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
	avk::cfg::compare_operation compare_operation = avk::cfg::compare_operation::greater_or_equal;

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
	
	template<IsValidPipeline T>
	T& get()
	{
		if (std::holds_alternative<T>(this->m_pipeline)) {
			return std::get<T>(this->m_pipeline);
		}

		throw_mismatch_error<T>();
	}

	template<IsValidPipeline T>
	const T& get() const
	{
		if (std::holds_alternative<T>(this->m_pipeline)) {
			return std::get<T>(this->m_pipeline);
		}

		throw_mismatch_error<T>();
	}
	
private:
	friend class ShaderManager;

	bool m_built = false;
	PipelineType m_type;
	ShaderMap m_shaders;
	AvkPipeline m_pipeline;
	std::size_t m_elements = 0;

	template <typename RequestedType>
	void throw_mismatch_error() const
	{
		if (std::holds_alternative<std::monostate>(this->m_pipeline) && !this->m_built) {
			throw std::runtime_error("Pipeline access error: The pipeline has not been built yet.");
		}

		std::string msg = "Pipeline access error: Requested type does not match active type.\n";
		msg += "Requested: " + std::string(typeid(RequestedType).name()) + "\n";

		std::visit([&](auto&& arg) {
			using ActiveType = std::decay_t<decltype(arg)>;
			msg += "Active:    " + std::string(typeid(ActiveType).name());
			}, m_pipeline);

		throw std::runtime_error(msg);
	}
};

class ShaderManager
{
public:
	ShaderManager() = default;
	ShaderManager(Root& root) : m_root(root) {
		SpirvTranslator::init(); // Maybe abstract into translator...
	}
	~ShaderManager()
	{
		SpirvTranslator::finalize();
	}

	void load(const std::string& shader_path, const std::string& compile_path);
	Pipeline& get_pipeline(const std::string& name);
	void build_pipeline(const std::string& name, avk::renderpass renderpass, const PipelineOptions& options = PipelineOptions(), uint32_t push_constant_size = 128);

private:
	Root& m_root;
	std::unordered_map<std::string, Pipeline> m_pipelines;

	void build_graphics_pipeline(Pipeline& pipeline, const PipelineOptions& options, avk::renderpass renderpass, uint32_t push_constant_size);
	void build_raytracing_pipeline(Pipeline& pipeline, const PipelineOptions& options, uint32_t push_constant_size);
	void build_compute_pipeline(Pipeline& pipeline, uint32_t push_constant_size);
};