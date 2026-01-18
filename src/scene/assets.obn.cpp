#include <native/scene/assets.obn.h>

AssetManager::AssetManager(Root& root) : m_root(root)
{
    auto white_texture = generic_texture({ 255, 255, 255, 255 });
    auto error_texture = generic_texture({ 255, 0, 255, 255 });
    auto normal_texture = generic_texture({ 128, 128, 255, 255 });

    auto def_ptr = std::make_shared<TextureAsset>(std::move(white_texture));
    auto err_ptr = std::make_shared<TextureAsset>(std::move(error_texture));
    auto nrm_ptr = std::make_shared<TextureAsset>(std::move(normal_texture));

    m_textures.try_emplace("OBSIDIAN_WHITE", def_ptr);
    m_textures.try_emplace("OBSIDIAN_ERROR", err_ptr);
    m_textures.try_emplace("OBSIDIAN_NORMAL", nrm_ptr);
}

auto AssetManager::mesh(const std::string& name) -> std::shared_ptr<MeshAsset>
{
    auto it = m_meshes.find(name);
    if (it != m_meshes.end())
    {
        return it->second;
    }

    LOG_S(WARNING) << "Mesh not found: " << name;
    return nullptr;
}

auto AssetManager::add_mesh(const std::string& name, const std::string& filepath) -> std::shared_ptr<MeshAsset>
{
    auto string_to_geometric_primitive = [](const std::string& type) {
        if (type == "OBSIDIAN_CUBE") return GeometricPrimitiveType::Cube;
        if (type == "OBSIDIAN_PLANE") return GeometricPrimitiveType::Plane;
        if (type == "OBSIDIAN_SPHERE") return GeometricPrimitiveType::Sphere;
        return GeometricPrimitiveType::Invalid;
    };

    auto geometry_type = string_to_geometric_primitive(filepath);
    GeometryData data = (geometry_type == GeometricPrimitiveType::Invalid) ? load_obj(filepath) : Primitives::generate(geometry_type);

    MeshAsset mesh;

    mesh.data = data;
    mesh.vertex_buffer = m_root.create_buffer(
        avk::memory_usage::host_visible,
        vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
        avk::vertex_buffer_meta::create_from_data(data.vertices)
    );
    mesh.vertex_buffer->fill(data.vertices.data(), 0);

    mesh.index_buffer = m_root.create_buffer(
        avk::memory_usage::host_visible,
        vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        avk::index_buffer_meta::create_from_data(data.indices)
    );
    mesh.index_buffer->fill(data.indices.data(), 0);
    mesh.index_count = static_cast<uint32_t>(data.indices.size());

    auto mesh_ptr = std::make_shared<MeshAsset>(std::move(mesh));
    return m_meshes.try_emplace(name, mesh_ptr).first->second;
}

auto AssetManager::has_mesh(const std::string& name) -> bool
{
    return m_meshes.contains(name);
}

auto AssetManager::texture(const std::string& name) -> std::shared_ptr<TextureAsset>
{
    auto it = m_textures.find(name);
    if (it != m_textures.end())
    {
        return it->second;
    }

    LOG_S(WARNING) << "Texture not found: " << name;
    return m_textures["OBSIDIAN_ERROR"];
}

auto AssetManager::add_texture(const std::string& name, const std::string& filepath) -> std::shared_ptr<TextureAsset>
{
    if (m_textures.find(name) != m_textures.end())
    {
        return m_textures[name];
    }

    int width, height, channels;
    stbi_uc* pixels = stbi_load(filepath.c_str(), &width, &height, &channels, 4);

    if (!pixels)
    {
        LOG_S(ERROR) << "Failed to load image file: " << filepath;
        return m_textures["OBSIDIAN_ERROR"];
    }

    try
    {
        auto staging = m_root.create_buffer(
            avk::memory_usage::host_visible,
            vk::BufferUsageFlagBits::eTransferSrc,
            avk::generic_buffer_meta::create_from_data(*pixels)
        );
        staging->fill(pixels, 0);

        stbi_image_free(pixels);

        auto image = m_root.create_image(width, height, OBN_DEFAULT_COLOR_FORMAT);
        upload_image_data(staging, image);

        auto view = m_root.create_image_view(image);
        auto sampler = m_root.create_sampler(avk::filter_mode::trilinear, avk::border_handling_mode::repeat);

        auto asset = std::make_shared<TextureAsset>();
        asset->image = std::move(image);
        asset->handle = m_root.create_image_sampler(std::move(view), std::move(sampler));
        asset->is_volume = false;

        m_textures[name] = asset;
        return asset;
    }
    catch (const std::exception& e)
    {
        if (pixels) stbi_image_free(pixels);
        LOG_S(ERROR) << "Vulkan failed during texture upload for '" << filepath << "': " << e.what();
        return m_textures["OBSIDIAN_ERROR"];
    }
}

auto AssetManager::add_texture(const std::string& name, const VolumeData& vd) -> std::shared_ptr<TextureAsset>
{
    if (m_textures.find(name) != m_textures.end())
    {
        return m_textures[name];
    }

    try
    {
        auto staging = m_root.create_buffer(avk::memory_usage::host_visible, vk::BufferUsageFlagBits::eTransferSrc, avk::generic_buffer_meta::create_from_data(vd.data));
        staging->fill(vd.data.data(), 0);

        auto image = m_root.create_image(
            vd.dimensions.x, vd.dimensions.y,
            { vd.vk_format(), vk::SampleCountFlagBits::e1}, 1,
            avk::memory_usage::device, avk::image_usage::general_image,
            [&vd](avk::image_t& img) {
                img.create_info().imageType = vk::ImageType::e3D;
                img.create_info().extent.depth = vd.dimensions.z;
            }
        );

        upload_image_data(staging, image);

        auto view = m_root.create_image_view(image);
        auto sampler = m_root.create_sampler(avk::filter_mode::trilinear, avk::border_handling_mode::clamp_to_edge);

        auto asset = std::make_shared<TextureAsset>();
        asset->image = std::move(image);
        asset->handle = m_root.create_image_sampler(std::move(view), std::move(sampler));
        asset->is_volume = true;

        m_textures[name] = asset;
        return asset;
    }
    catch (const std::exception& e)
    {
        LOG_S(ERROR) << "Failed to upload volume texture: " << e.what();
        return nullptr;
    }
}

auto AssetManager::has_texture(const std::string& name) -> bool
{
    return m_textures.contains(name);
}

auto AssetManager::material(const std::string& name) -> std::shared_ptr<MaterialAsset>
{
    auto it = m_materials.find(name);
    if (it != m_materials.end())
    {
        return it->second;
    }

    LOG_S(WARNING) << "Material not found: " << name;
    return nullptr;
}

auto AssetManager::add_material(const std::string& name, const MaterialAsset& material) -> std::shared_ptr<MaterialAsset>
{
    auto mat = std::make_shared<MaterialAsset>(material);
    mat->name = name;

    if (!material.albedo_map)     mat->albedo_map     = m_textures["OBSIDIAN_WHITE"];
    if (!material.roughness_map)  mat->roughness_map  = m_textures["OBSIDIAN_WHITE"];
    if (!material.normal_map)     mat->normal_map     = m_textures["OBSIDIAN_NORMAL"];
    if (!material.ao_map)         mat->ao_map         = m_textures["OBSIDIAN_WHITE"];

    MaterialUniforms ubo_data;
    ubo_data.base_color = mat->base_color;
    ubo_data.params = glm::vec4(mat->metallic, mat->roughness, 0.0f, 0.0f);

    mat->ubo = m_root.create_buffer(
        avk::memory_usage::host_visible,
        vk::BufferUsageFlagBits::eUniformBuffer,
        avk::generic_buffer_meta::create_from_data(ubo_data)
    );
    mat->ubo->fill(&ubo_data, 0);

    m_materials[name] = mat;
    return mat;
}

auto AssetManager::has_material(const std::string& name) -> bool
{
    return m_materials.contains(name);
}

auto AssetManager::load_obj(const std::string& filepath) -> GeometryData
{
    GeometryData data;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str());

    if (!warn.empty()) { /* LOG_S(WARNING) << warn; */ }
    if (!err.empty()) { throw std::runtime_error("OBJ Load Error: " + err); }
    if (!ret) { throw std::runtime_error("Failed to parse OBJ file"); }

    // Map to track unique vertices: tinyobj index -> your vertex buffer index
    std::unordered_map<tinyobj::index_t, uint32_t, IndexHash, IndexEqual> unique_vertices;
    bool normals_loaded = !attrib.normals.empty();

    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            // 1. Check if we've already seen this exact vertex combination
            if (unique_vertices.count(index) == 0)
            {
                Vertex vertex{};

                // --- Position ---
                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                // --- Normal ---
                // If normals exist in file, load them. Otherwise init to 0.
                if (normals_loaded && index.normal_index >= 0) {
                    vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]
                    };
                }
                else {
                    vertex.normal = { 0.0f, 0.0f, 0.0f };
                }

                // --- UV ---
                if (index.texcoord_index >= 0) {
                    vertex.uv = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                    };
                }

                // Push new unique vertex
                uint32_t new_index = static_cast<uint32_t>(data.vertices.size());
                unique_vertices[index] = new_index;
                data.vertices.push_back(vertex);
                data.indices.push_back(new_index);
            }
            else
            {
                // Reuse existing vertex index
                data.indices.push_back(unique_vertices[index]);
            }
        }
    }

    // --- AUTO-GENERATE SMOOTH NORMALS IF MISSING ---
    if (!normals_loaded)
    {
        for (size_t i = 0; i < data.indices.size(); i += 3)
        {
            // Get indices for the triangle
            uint32_t i0 = data.indices[i];
            uint32_t i1 = data.indices[i + 1];
            uint32_t i2 = data.indices[i + 2];

            // Get positions (using glm for convenience, replace with your math if needed)
            glm::vec3 p0(data.vertices[i0].pos[0], data.vertices[i0].pos[1], data.vertices[i0].pos[2]);
            glm::vec3 p1(data.vertices[i1].pos[0], data.vertices[i1].pos[1], data.vertices[i1].pos[2]);
            glm::vec3 p2(data.vertices[i2].pos[0], data.vertices[i2].pos[1], data.vertices[i2].pos[2]);

            // Calculate Face Normal
            glm::vec3 edge1 = p1 - p0;
            glm::vec3 edge2 = p2 - p0;
            glm::vec3 face_normal = glm::cross(edge1, edge2);

            // Accumulate (no map needed anymore!)
            // We just add the face normal to the shared vertex data.
            auto add_normal = [&](uint32_t idx) {
                data.vertices[idx].normal[0] += face_normal.x;
                data.vertices[idx].normal[1] += face_normal.y;
                data.vertices[idx].normal[2] += face_normal.z;
                };

            add_normal(i0);
            add_normal(i1);
            add_normal(i2);
        }

        struct PosKey {
            float x, y, z;
            bool operator==(const PosKey& o) const {
                return x == o.x && y == o.y && z == o.z; // exact float match is usually fine for deduplicating raw OBJ data
            }
        };

        struct PosHash {
            std::size_t operator()(const PosKey& k) const {
                // Simple hash combining x, y, z
                return std::hash<float>()(k.x) ^ (std::hash<float>()(k.y) << 1) ^ (std::hash<float>()(k.z) << 2);
            }
        };

        // 1. Group indices by spatial position
        std::unordered_map<PosKey, std::vector<uint32_t>, PosHash> positional_groups;

        for (size_t i = 0; i < data.vertices.size(); ++i) {
            const auto& pos = data.vertices[i].pos;
            positional_groups[{pos[0], pos[1], pos[2]}].push_back(static_cast<uint32_t>(i));
        }

        // 2. Average the normals within each group
        for (auto& [key, indices] : positional_groups) {
            if (indices.size() <= 1) continue; // No split here, skip

            // Sum up all normals at this position
            float sum_x = 0, sum_y = 0, sum_z = 0;
            for (uint32_t idx : indices) {
                sum_x += data.vertices[idx].normal[0];
                sum_y += data.vertices[idx].normal[1];
                sum_z += data.vertices[idx].normal[2];
            }

            // Apply the sum back to all vertices at this position
            // (Normalization later will handle the division)
            for (uint32_t idx : indices) {
                data.vertices[idx].normal = { sum_x, sum_y, sum_z };
            }
        }

        // Normalize
        for (auto& v : data.vertices) {
            glm::vec3 n(v.normal[0], v.normal[1], v.normal[2]);
            if (glm::length(n) > OBN_EPSILON) { // Avoid NaN
                n = glm::normalize(n);
                v.normal = { n.x, n.y, n.z };
            }
        }
    }

    return data;
}

auto AssetManager::load_vol(const std::string& filepath, const std::array<int, 3>& dims, const std::string& voxel_type, size_t skip_bytes) -> VolumeData
{
    VolumeData vd;
    vd.dimensions = glm::ivec3({ dims[0], dims[1], dims[2] });
    vd.format = voxel_type;

    size_t total_voxels = static_cast<size_t>(dims[0]) * dims[1] * dims[2];
    size_t total_bytes = total_voxels * vd.bytes_per_voxel();

    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open volume file: " + filepath);
    }

    if (skip_bytes > 0)
    {
        file.seekg(skip_bytes, std::ios::beg);
    }

    vd.data.resize(total_bytes);

    if (!file.read(reinterpret_cast<char*>(vd.data.data()), total_bytes))
    {
        throw std::runtime_error("File too small. Expected " + std::to_string(total_bytes) + " bytes after skip, but hit EOF.");
    }

    if (voxel_type == "uint12")
    {
        uint16_t* voxel_ptr = reinterpret_cast<uint16_t*>(vd.data.data());

        for (size_t i = 0; i < total_voxels; ++i)
        {
            voxel_ptr[i] = voxel_ptr[i] << 4; // 4095 * 16 = 65520 ~ 65536
        }
    }

    return vd;
}

auto AssetManager::generic_texture(const std::array<uint8_t, 4>& rgba) -> TextureAsset
{
    auto staging = m_root.create_buffer(
        avk::memory_usage::host_visible,
        vk::BufferUsageFlagBits::eTransferSrc,
        avk::generic_buffer_meta::create_from_data(rgba)
    );
    staging->fill(rgba.data(), 0);

    auto image = m_root.create_image(1, 1, OBN_DEFAULT_COLOR_FORMAT);
    upload_image_data(staging, image);

    auto view = m_root.create_image_view(image);
    auto sampler = m_root.create_sampler(avk::filter_mode::nearest_neighbor, avk::border_handling_mode::repeat);

    TextureAsset asset;
    asset.image = std::move(image);
    asset.handle = m_root.create_image_sampler(std::move(view), std::move(sampler));

    return asset;
}

auto AssetManager::upload_image_data(avk::buffer& staging, avk::image& target) -> void
{
    vk::Device device = m_root.device();
    vk::Queue queue = m_root.queue().handle();
    uint32_t q_family_index = m_root.queue().family_index();
    vk::Image image_handle = target->handle();

    auto img_info = target->create_info();
    vk::Extent3D extent = img_info.extent;
    vk::ImageType img_type = img_info.imageType;
    uint32_t mip_levels = img_info.mipLevels ? img_info.mipLevels : 1;
    uint32_t array_layers = img_info.arrayLayers ? img_info.arrayLayers : 1;
    uint32_t copy_layer_count = (img_type == vk::ImageType::e3D) ? 1u : array_layers;

    vk::CommandPoolCreateInfo pool_info;
    pool_info.flags = vk::CommandPoolCreateFlagBits::eTransient;
    pool_info.queueFamilyIndex = q_family_index;

    vk::CommandPool cmd_pool = device.createCommandPool(pool_info);

    try
    {
        // Allocate one primary command buffer
        vk::CommandBufferAllocateInfo alloc_info;
        alloc_info.commandPool = cmd_pool;
        alloc_info.level = vk::CommandBufferLevel::ePrimary;
        alloc_info.commandBufferCount = 1;

        vk::CommandBuffer cmd = device.allocateCommandBuffers(alloc_info).front();

        // Begin the command buffer as one-time submit
        vk::CommandBufferBeginInfo begin_info;
        begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
        cmd.begin(begin_info);

        // Prepare an image memory barrier to transition undefined -> transfer dst
        vk::ImageSubresourceRange full_range{
            vk::ImageAspectFlagBits::eColor,
            0,
            mip_levels,
            0,
            copy_layer_count
        };

        vk::ImageMemoryBarrier pre_barrier{};
        pre_barrier.oldLayout = vk::ImageLayout::eUndefined;
        pre_barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
        pre_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        pre_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        pre_barrier.image = image_handle;
        pre_barrier.subresourceRange = full_range;
        pre_barrier.srcAccessMask = vk::AccessFlags();
        pre_barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        cmd.pipelineBarrier(
            vk::PipelineStageFlagBits::eTopOfPipe,
            vk::PipelineStageFlagBits::eTransfer,
            vk::DependencyFlags(),
            0, nullptr, 0, nullptr, 1, &pre_barrier
        );

        vk::BufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource = { vk::ImageAspectFlagBits::eColor, 0, 0, copy_layer_count };
        region.imageOffset = vk::Offset3D{ 0, 0, 0 };
        region.imageExtent = extent;

        cmd.copyBufferToImage(staging->handle(), image_handle, vk::ImageLayout::eTransferDstOptimal, 1, &region);

        ImageBarrier::transition(cmd, image_handle)
            .from(vk::ImageLayout::eTransferDstOptimal).as(vk::AccessFlagBits::eTransferWrite, vk::PipelineStageFlagBits::eTransfer)
            .to(vk::ImageLayout::eShaderReadOnlyOptimal).as(vk::AccessFlagBits::eShaderRead, vk::PipelineStageFlagBits::eFragmentShader)
            .commit();

        cmd.end();

        // Submit and wait (simple single-use upload)
        vk::SubmitInfo submit_info;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &cmd;

        queue.submit(submit_info, nullptr);
        queue.waitIdle();
    }
    catch (...)
    {
        // Ensure command pool is destroyed on exception path
        device.destroyCommandPool(cmd_pool);
        throw;
    }

    // Destroy the transient pool (frees the command buffers too)
    device.destroyCommandPool(cmd_pool);
}

GeometryData Primitives::generate(GeometricPrimitiveType type)
{
    GeometryData object{};

    switch (type)
    {
    case GeometricPrimitiveType::Plane:
        object = _generate_plane_gm();
        break;
    case GeometricPrimitiveType::Cube:
        object = _generate_cube_gm();
        break;
    case GeometricPrimitiveType::Sphere:
        object = _generate_sphere_gm(32, 16);
        break;
    }

    return object;
}

GeometryData Primitives::_generate_plane_gm()
{
    GeometryData data;

    data.vertices = {
        // Position (x,y,z)    // Normal (x,y,z)   // UV (u,v)
        {{-0.5f, 0.0f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{ 0.5f, 0.0f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{ 0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}
    };

    data.indices = { 0, 1, 2, 2, 3, 0 };

    return data;
}

GeometryData Primitives::_generate_cube_gm()
{
    GeometryData data;

    auto add_face = [&](const glm::vec3& normal, const glm::vec3& up, const glm::vec3& right) {
        glm::vec3 center = normal * 0.5f;
        glm::vec3 bl = center - right * 0.5f - up * 0.5f;
        glm::vec3 br = center + right * 0.5f - up * 0.5f;
        glm::vec3 tr = center + right * 0.5f + up * 0.5f;
        glm::vec3 tl = center - right * 0.5f + up * 0.5f;

        uint32_t startIdx = static_cast<uint32_t>(data.vertices.size());

        // Vertices, Normals, UV
        data.vertices.push_back({ bl, normal, {0.0f, 0.0f} });
        data.vertices.push_back({ br, normal, {1.0f, 0.0f} });
        data.vertices.push_back({ tr, normal, {1.0f, 1.0f} });
        data.vertices.push_back({ tl, normal, {0.0f, 1.0f} });

        // Indices
        data.indices.push_back(startIdx + 0);
        data.indices.push_back(startIdx + 1);
        data.indices.push_back(startIdx + 2);
        data.indices.push_back(startIdx + 2);
        data.indices.push_back(startIdx + 3);
        data.indices.push_back(startIdx + 0);
        };

    add_face({ 0, 0, 1 }, { 0, 1, 0 }, { 1, 0, 0 });
    add_face({ 0, 0,-1 }, { 0, 1, 0 }, { -1, 0, 0 });
    add_face({ 1, 0, 0 }, { 0, 1, 0 }, { 0, 0,-1 });
    add_face({ -1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 });
    add_face({ 0, 1, 0 }, { 0, 0,-1 }, { 1, 0, 0 });
    add_face({ 0,-1, 0 }, { 0, 0, 1 }, { 1, 0, 0 });

    return data;
}

GeometryData Primitives::_generate_sphere_gm(int sectors, int stacks)
{
    GeometryData data;
    float radius = 0.5f;
    double sectorStep = OBN_TWOPI / sectors;
    double stackStep = OBN_PI / stacks;

    for (int i = 0; i <= stacks; ++i)
    {
        double stackAngle = OBN_PI * 0.5f - i * stackStep;
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);

        for (int j = 0; j <= sectors; ++j)
        {
            float sectorAngle = j * sectorStep;

            Vertex vertex{};
            vertex.pos.x = xy * cosf(sectorAngle);
            vertex.pos.y = xy * sinf(sectorAngle);
            vertex.pos.z = z;

            // Normal
            vertex.normal = glm::normalize(vertex.pos);

            // UV Coordinates
            vertex.uv.x = (float)j / sectors;
            vertex.uv.y = (float)i / stacks;

            data.vertices.push_back(vertex);
        }
    }

    for (int i = 0; i < stacks; ++i)
    {
        int k1 = i * (sectors + 1);
        int k2 = k1 + sectors + 1;

        for (int j = 0; j < sectors; ++j, ++k1, ++k2)
        {
            if (i != 0)
            {
                data.indices.push_back(k1);
                data.indices.push_back(k2);
                data.indices.push_back(k1 + 1);
            }

            if (i != (stacks - 1))
            {
                data.indices.push_back(k1 + 1);
                data.indices.push_back(k2);
                data.indices.push_back(k2 + 1);
            }
        }
    }

    return data;
}
