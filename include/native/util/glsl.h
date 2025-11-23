#pragma once

#include <native/core/obsidian/include.h>

struct SpirvTranslator
{
	static void init();
	static void finalize();
	static void init_resources(TBuiltInResource& resources);
	static EShLanguage find_language(const vk::ShaderStageFlagBits shader_type);
	static bool glsl_to_spv(const vk::ShaderStageFlagBits shader_type, const char* pshader, std::vector<unsigned int>& spirv);
};