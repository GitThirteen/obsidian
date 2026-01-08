#include <native/platform/ui.h>
#include <native/graphics/resources.obn.h>

auto UserInterface::initialize(Root& root, GLFWwindow* window, ResourceManager& resources) const -> void
{
    if (!window)
    {
        LOG_S(ERROR) << "GLFW window not set in UI! Aborting.";
        exit(0);
    }

    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = root.instance();
    init_info.PhysicalDevice = root.physical_device();
    init_info.Device = root.device();
    init_info.QueueFamily = root.queue().family_index();
    init_info.Queue = root.queue().handle();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = resources.descriptor_pool().handle();
    init_info.MinImageCount = 2;
    init_info.ImageCount = 2;
    init_info.Allocator = VK_NULL_HANDLE;
    init_info.PipelineInfoMain.Subpass = 0;
    init_info.UseDynamicRendering = true;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.CheckVkResultFn = check_vk_result;

    const VkFormat color_formats[] = { static_cast<VkFormat>(OBN_DEFAULT_COLOR_FORMAT) };

    VkPipelineRenderingCreateInfoKHR rendering_create_info = {};
    rendering_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    rendering_create_info.colorAttachmentCount = 1;
    rendering_create_info.pColorAttachmentFormats = color_formats;

    init_info.PipelineInfoMain.PipelineRenderingCreateInfo = rendering_create_info;

    ImGui_ImplVulkan_Init(&init_info);
}

auto UserInterface::render(const vk::CommandBuffer& cb, const vk::ImageView& target_view) -> void
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static float f = 0.0f;
    static int counter = 0;
    ImGui::Begin("Obsidian Controls");
    ImGui::Text("Render Settings");
    ImGui::ColorEdit3("Clear Color", m_clear_color.data());
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();

    ImGui::Begin("Transfer Functions");

    // Particle Color (c_tf)
    edit_color("Particle Color (Reflective)", tfs.particle_colors);

    // Opacity (alpha_tf)
    edit_scalar("Opacity (Density)", tfs.opacities, 0.0f, 1.0f);

    // Medium Color (m_tf)
    edit_color("Medium Color (Absorptive)", tfs.medium_colors);

    // Refraction (eta_tf)
    edit_scalar("Refractive Index (IOR)", tfs.iors, 1.0f, 3.0f);

    ImGui::End();
    ImGui::Render();

    ImDrawData* draw_data = ImGui::GetDrawData();

    vk::RenderingAttachmentInfo color_attachment{};
    color_attachment.imageView = target_view;
    color_attachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    color_attachment.loadOp = vk::AttachmentLoadOp::eLoad;
    color_attachment.storeOp = vk::AttachmentStoreOp::eStore;

    vk::RenderingInfo rendering_info{};
    rendering_info.renderArea = vk::Rect2D{ {0, 0}, {static_cast<uint32_t>(draw_data->DisplaySize.x), static_cast<uint32_t>(draw_data->DisplaySize.y)} };
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &color_attachment;

    cb.beginRendering(rendering_info);

    ImGui_ImplVulkan_RenderDrawData(draw_data, cb);

    cb.endRendering();
}

auto UserInterface::destroy() -> void
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void UserInterface::check_vk_result(VkResult error)
{
    if (error == VK_SUCCESS)
    {
        return;
    }

    LOG_S(ERROR) << "[vulkan] Error: VkResult = " << error;

    if (error < 0)
    {
        abort();
    }
}

auto UserInterface::edit_scalar(const char* label, std::vector<ScalarPoint>& points, float min_val, float max_val) -> void
{
    ImGui::PushID(label);
    ImGui::Text("%s", label);


    const int sample_count = 100;
    float values[sample_count];

    for (int i = 0; i < sample_count; ++i)
    {
        float t = i / (float)(sample_count - 1);
        float val = points.front().value; // default

        for (size_t k = 0; k < points.size() - 1; ++k)
        {
            if (t >= points[k].pos && t <= points[k + 1].pos)
            {
                float range = points[k + 1].pos - points[k].pos;
                float weight = (t - points[k].pos) / range;
                val = points[k].value * (1.0f - weight) + points[k + 1].value * weight;
                break;
            }
        }
        values[i] = val;
    }
    ImGui::PlotLines("##Curve", values, sample_count, 0, nullptr, min_val, max_val, ImVec2(0, 80));

    if (ImGui::Button("Add Point")) points.push_back({ 0.5f, (min_val + max_val) * 0.5f });

    for (size_t i = 0; i < points.size(); ++i)
    {
        ImGui::PushID((int) i);
        ImGui::SetNextItemWidth(60);

        if (ImGui::DragFloat("Pos", &points[i].pos, 0.01f, 0.0f, 1.0f))
        {
            sort_points(points);
        }

        ImGui::SameLine();
        ImGui::SetNextItemWidth(60);
        ImGui::DragFloat("Val", &points[i].value, 0.01f, min_val, max_val);
        ImGui::SameLine();

        if (ImGui::Button("X"))
        {
            points.erase(points.begin() + i);
            i--; // adjust index
        }

        ImGui::PopID();
    }

    ImGui::PopID();
    ImGui::Separator();
}

auto UserInterface::edit_color(const char* label, std::vector<ColorPoint>& points) -> void
{
    ImGui::PushID(label);
    ImGui::Text("%s", label);

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    float width = ImGui::GetContentRegionAvail().x;
    float height = 20.0f;

    for (int i = 0; i < 100; i++)
    {
        float t = i / 100.0f;
        ImVec4 col = points.front().color;
        for (size_t k = 0; k < points.size() - 1; ++k)
        {
            if (t >= points[k].pos && t <= points[k + 1].pos)
            {
                float range = points[k + 1].pos - points[k].pos;
                float w = (t - points[k].pos) / range;

                col.x = points[k].color.x * (1 - w) + points[k + 1].color.x * w;
                col.y = points[k].color.y * (1 - w) + points[k + 1].color.y * w;
                col.z = points[k].color.z * (1 - w) + points[k + 1].color.z * w;
                break;
            }
        }
        draw_list->AddRectFilled(ImVec2(p.x + t * width, p.y), ImVec2(p.x + (t + 0.01f) * width, p.y + height), ImGui::ColorConvertFloat4ToU32(col));
    }
    ImGui::Dummy(ImVec2(width, height));

    if (ImGui::Button("Add Point"))
    {
        points.push_back({ 0.5f, {1,1,1,1} });
    }

    for (size_t i = 0; i < points.size(); ++i)
    {
        ImGui::PushID((int) i);
        ImGui::SetNextItemWidth(60);

        if (ImGui::DragFloat("Pos", &points[i].pos, 0.01f, 0.0f, 1.0f))
        {
            sort_points(points);
        }
        
        ImGui::SameLine();
        ImGui::ColorEdit3("##Col", (float*)&points[i].color, ImGuiColorEditFlags_NoInputs);
        ImGui::SameLine();

        if (ImGui::Button("X"))
        {
            points.erase(points.begin() + i);
            i--;
        }

        ImGui::PopID();
    }

    ImGui::PopID();
    ImGui::Separator();
}
