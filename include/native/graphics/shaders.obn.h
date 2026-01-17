#pragma once

#include <native/core/include.h>
#include <native/core/root.h>
#include <native/graphics/compiler.h>
#include <native/graphics/resources.obn.h>
#include <native/scene/assets.obn.h>
#include <native/utils/templates.h>
#include <native/utils/io.h>

namespace avk
{
	template<> inline vk::Format format_for<glm::vec2>()
	{
		return vk::Format::eR32G32Sfloat;
	}

	template<> inline vk::Format format_for<glm::vec3>()
	{
		return vk::Format::eR32G32B32Sfloat;
	}

	template<> inline vk::Format format_for<glm::vec4>()
	{
		return vk::Format::eR32G32B32A32Sfloat;
	}
}

/**
 * @brief Describes the type of shader to be used in a pipeline.
 */
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

/**
 * @brief Possible pipeline types.
 */
enum class PipelineType
{
	Graphics,
	RayTracing,
	Compute
};

struct ShaderBindings
{
	enum class BindingType
	{
		Unknown,
		UniformBuffer,			// UBOs
		UniformBufferMember,	// UBO members
		StorageBuffer,			// SSBOs
		CombinedImageSampler,	// Textures, Samplers
		SampledImage,			// Read-only textures
		StorageImage,			// Compute or RW textures
		AccelerationStructure	// Ray tracing acceleration structures
	};

	std::string name;
	uint32_t binding_index;	// Index of the entire buffer
	uint32_t size;			// Padded size of the field (16 for vec3, 8 for vec2, etc.)
	uint32_t offset;		// Where to start reading memory from the buffer
	uint32_t set_index;		// Index of the descriptor set containing this buffer, we'll assume there is only one set
	uint32_t count = 1;		// Number of array elements (1 if not an array)
	BindingType type = BindingType::Unknown;
	vk::ShaderStageFlags stages = vk::ShaderStageFlagBits::eAll;

	auto to_vk_descriptor() const -> vk::DescriptorType;
	static auto create_from_descriptor(const SpvReflectDescriptorBinding& descriptor, const vk::ShaderStageFlagBits stage) -> ShaderBindings;
};

/**
 * @brief Struct holding pipeline configuration parameters.
 */
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

	static auto create_default() -> PipelineOptions
	{
		return PipelineOptions();
	}

	static auto create_wireframe() -> PipelineOptions
	{
		PipelineOptions options;
		options.polygon_mode = avk::cfg::polygon_drawing_mode::line;
		options.cull_mode = avk::cfg::culling_mode::disabled;
		return options;
	}

	static auto create_transparent() -> PipelineOptions
	{
		PipelineOptions options;
		options.depth_write = false;
		options.alpha_blending = true;
		return options;
	}
};

/**
 * @brief Represents a key used for identifying pipelines based on their configuration.
 */
struct PipelineKey
{
	std::string name;
	std::vector<vk::Format> colors;
	vk::Format depth;

	bool operator==(const PipelineKey& other) const
	{
		return name == other.name && colors == other.colors && depth == other.depth;
	}
};

struct PipelineKeyHash
{
	std::size_t operator()(const PipelineKey& key) const
	{
		std::size_t h = std::hash<std::string>{}(key.name);
		for (auto f : key.colors) h ^= std::hash<int>{}(static_cast<int>(f));
		h ^= std::hash<int>{}(static_cast<int>(key.depth));
		return h;
	}
};

struct PipelineBlueprint
{
	std::string name;
	PipelineType type;
	std::map<ShaderType, std::vector<avk::shader_info>> shaders;

	std::vector<ShaderBindings> bindings;
	std::unordered_map<std::string, ShaderBindings> bindings_by_name;

	uint32_t push_constant_size = 0;
	vk::ShaderStageFlags push_constant_stages;

	auto add_shader(const ShaderType type, const vk::ShaderStageFlagBits stage, const avk::shader_info& shader_info, const std::vector<uint32_t>& spirv_binary) -> void;
	auto has_shader(const ShaderType type) const -> bool;
	auto as_binding_data() const -> std::vector<avk::binding_data>;
};

struct Pipeline
{
	using ShaderMap = std::unordered_map<ShaderType, std::vector<avk::shader_info>>;
	using AvkPipeline = std::variant<std::monostate, avk::graphics_pipeline, avk::ray_tracing_pipeline, avk::compute_pipeline>;
	
	Pipeline() = default;

	auto add_shader(ShaderType type, const avk::shader_info& shader_info) -> void;
	auto set_type(PipelineType type) -> void;
	auto shaders(ShaderType type) const -> std::span<const avk::shader_info>;
	auto elements() const -> std::size_t;
	auto type() const -> PipelineType;
	auto has_shader(ShaderType type) const -> bool;
	auto bind_into(avk::command_buffer_t& cmd_buffer) const -> void;
	auto layout_handle() const -> vk::PipelineLayout;
	auto make_descriptor_set(avk::descriptor_pool& pool, uint32_t set_index = 0) const -> vk::DescriptorSet;

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

	PipelineType m_type;
	ShaderMap m_shaders;
	AvkPipeline m_pipeline;
	std::size_t m_elements = 0;

	template <typename RequestedType>
	auto throw_mismatch_error() const -> void
	{
		if (std::holds_alternative<std::monostate>(this->m_pipeline)) {
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
	ShaderManager(Root& root) : m_root(root)
	{
		SpirvTranslator::init();
	}
	~ShaderManager()
	{
		SpirvTranslator::finalize();
	}

	/**
	 * @brief Loads shards from the config and stores its metadata into a blueprint, which are then warmed up to pipeline objects when requested by a scene.
	 * @param shader_path The path where the shader source files are located.
	 * @param compile_path The path where the compiled SPIR-V binaries should be stored.
	 */
	auto load(const std::string& shader_path, const std::string& compile_path) -> void;
	auto pipeline(const std::string& name, const Shard& shard, const PipelineOptions& options = PipelineOptions::create_default()) -> Pipeline&;
	auto pipeline(const std::string& name, const std::vector<ShardMetadata>& color_attachments, const ShardMetadata& depth_attachment, const PipelineOptions& options) -> Pipeline&;
	auto blueprint(const std::string& name) const -> const PipelineBlueprint&;

private:
	Root& m_root;
	std::unordered_map<std::string, PipelineBlueprint> m_blueprints;
	std::unordered_map<PipelineKey, Pipeline, PipelineKeyHash> m_pipelines;

	auto build_graphics_pipeline(Pipeline& pipeline, const PipelineBlueprint& blueprint, const PipelineOptions& options, const std::vector<vk::Format>& color_formats, const vk::Format depth_format) -> void;
	auto build_raytracing_pipeline(Pipeline& pipeline, const PipelineBlueprint& blueprint, const PipelineOptions& options) -> void;
	auto build_compute_pipeline(Pipeline& pipeline, const PipelineBlueprint& blueprint) -> void;
};