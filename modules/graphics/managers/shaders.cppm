module;
#include <vulkan/vulkan_raii.hpp>
#include <slang.h>
#include <slang-com-ptr.h>
#include <native/macros.h>

// ================================ //
// GRAPHICS - SHADER MANAGER MODULE //
// ================================ //

export module Obsidian.Graphics.ShaderManager;

import std;
//import Obsidian.Core;
import Obsidian.Core.Manager;
import Obsidian.Core.Logger;
import Obsidian.Core.Result;
import Obsidian.Graphics.Pipeline;
import Obsidian.Graphics.Shard;

EXPORT(obsidian)

class ShaderManager : public Manager
{
public:
    ShaderManager() = default;

    auto initialize() -> Result<void> override
    {
        log::info("Initializing Slang compiler session...");
        
        if (SLANG_FAILED(slang::createGlobalSession(m_slang_session.writeRef()))) {
            return std::unexpected<std::string>("Failed to create Slang global session.");
        }
        
        return {};
    }

    auto pipeline(const vk::raii::Device& device, const std::string& name, const std::vector<ShardMetadata>& color_attachments, const ShardMetadata& depth_attachment, const PipelineOptions::Type options_type = PipelineOptions::Type::Default) -> Result<Pipeline*> 
    {
        std::vector<vk::Format> color_formats;
        for (const auto& attachment : color_attachments)
        {
            color_formats.push_back(attachment.format);
        }

        vk::Format depth_format = depth_attachment.format;
        const PipelineKey key{ name, color_formats, depth_format, options_type };

        // Return cached pipeline if found
        if (m_pipelines.contains(key))
        {
            return &m_pipelines[key];
        }

        // Otherwise, we build it!
        auto options = (options_type == PipelineOptions::Type::Transparent) ? PipelineOptions::create_transparent()
                        : (options_type == PipelineOptions::Type::Wireframe) ? PipelineOptions::create_wireframe()
                        : (options_type == PipelineOptions::Type::Additive) ? PipelineOptions::create_transparent("additive")
                        : PipelineOptions::create_default();

        return build_graphics_pipeline(device, key, options)
            .map([&](Pipeline& new_pipe) {
                m_pipelines[key] = std::move(new_pipe);
                return &m_pipelines[key];
            });
    }

    auto compile_shader(const vk::raii::Device& device, std::string_view file_path, std::string_view entry_point, SlangStage stage) -> Result<vk::raii::ShaderModule>
    {
        Slang::ComPtr<slang::ISession> session;
        slang::SessionDesc session_desc = {};
        slang::TargetDesc target_desc = {};
        target_desc.format = SLANG_SPIRV;
        target_desc.profile = m_slang_session->findProfile("glsl_460"); // Target modern Vulkan
        
        session_desc.targets = &target_desc;
        session_desc.targetCount = 1;

        if (SLANG_FAILED(m_slang_session->createSession(session_desc, session.writeRef())))
        {
            return std::unexpected<std::string>("Failed to create Slang compilation session.");
        }

        Slang::ComPtr<slang::ICompileRequest> request;
        session->createCompileRequest(request.writeRef());

        // Load the file
        int translation_unit = request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, nullptr);
        request->addTranslationUnitSourceFile(translation_unit, file_path.data());

        // Set the entry point
        int entry_point_index = request->addEntryPoint(translation_unit, entry_point.data(), stage);

        // Compile
        const int any_errors = request->compile();
        if (const char* diagnostics = request->getDiagnosticOutput(); diagnostics && diagnostics[0])
        {
            if (any_errors) return std::unexpected<std::string>(std::format("Slang Compilation Error ({}): {}", file_path, diagnostics));
            else log::warn("Slang Compilation Warning ({}): {}", file_path, diagnostics);
        }

        // Retrieve SPIR-V binary
        size_t spirv_size = 0;
        const void* spirv_data = request->getEntryPointCode(entry_point_index, &spirv_size);

        if (!spirv_data || spirv_size == 0)
        {
            return std::unexpected<std::string>("Slang returned empty SPIR-V binary.");
        }

        // Instantly generate Vulkan Shader Module
        try {
            vk::ShaderModuleCreateInfo create_info;
            create_info.setCodeSize(spirv_size);
            create_info.setPCode(static_cast<const uint32_t*>(spirv_data));
            
            return vk::raii::ShaderModule(device, create_info);
        } catch (const std::exception& e) {
            return std::unexpected<std::string>(std::format("Failed to create Vulkan Shader Module: {}", e.what()));
        }
    }
    
    auto build_pipeline(const vk::raii::Device& device, const vk::raii::ShaderModule& vert, const vk::raii::ShaderModule& frag) -> Result<vk::raii::Pipeline>
    {
        return std::unexpected<std::string>("Pipeline building not fully implemented yet.");
    }

private:
    Slang::ComPtr<slang::IGlobalSession> m_slang_session;
    std::unordered_map<PipelineKey, Pipeline, PipelineKeyHash> m_pipelines;

    auto build_graphics_pipeline(const vk::raii::Device& device, const PipelineKey& key, const PipelineOptions& options) -> Result<Pipeline>
    {
        return std::unexpected<std::string>("Failed to build graphics pipeline: Not implemented yet.");
    }
};

EXPORT_END