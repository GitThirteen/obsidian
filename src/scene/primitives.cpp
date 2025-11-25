#include <native/scene/primitives.h>

std::shared_ptr<SceneObject> Primitives::generate(Root& root, GeometricPrimitiveType type)
{
	auto counter = ++internal_counter[static_cast<int>(type)];

	GeometryData object{};
	std::string_view base_name = "";

	switch (type)
	{
	case GeometricPrimitiveType::Plane:
		object = _generate_plane_gm();
		base_name = "Plane";
		break;
	case GeometricPrimitiveType::Cube:
		object = _generate_cube_gm();
		base_name = "Cube";
		break;
	case GeometricPrimitiveType::Sphere:
		object = _generate_sphere_gm(32, 16);
		base_name = "Sphere";
		break;
	}

	return std::make_shared<SceneObject>(
		root, object, std::format("{}{}", base_name, counter)
	);
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
		data.vertices.push_back({ bl, normal, {0.0f, 0.0f} }); // 0: Bottom-Left
		data.vertices.push_back({ br, normal, {1.0f, 0.0f} }); // 1: Bottom-Right
		data.vertices.push_back({ tr, normal, {1.0f, 1.0f} }); // 2: Top-Right
		data.vertices.push_back({ tl, normal, {0.0f, 1.0f} }); // 3: Top-Left

		// Indices
		data.indices.push_back(startIdx + 0);
		data.indices.push_back(startIdx + 1);
		data.indices.push_back(startIdx + 2);
		data.indices.push_back(startIdx + 2);
		data.indices.push_back(startIdx + 3);
		data.indices.push_back(startIdx + 0);
	};

	add_face({ 0, 0, 1 }, { 0, 1, 0 }, { 1, 0, 0 });
	add_face({ 0, 0,-1 }, { 0, 1, 0 }, {-1, 0, 0 });
	add_face({ 1, 0, 0 }, { 0, 1, 0 }, { 0, 0,-1 });
	add_face({-1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 });
	add_face({ 0, 1, 0 }, { 0, 0,-1 }, { 1, 0, 0 });
	add_face({ 0,-1, 0 }, { 0, 0, 1 }, { 1, 0, 0 });

	return data;
}

GeometryData Primitives::_generate_sphere_gm(int sectors, int stacks)
{
	GeometryData data;
	float radius = 0.5f;
	float sectorStep = 2 * glm::pi<float>() / sectors;
	float stackStep = glm::pi<float>() / stacks;

	for (int i = 0; i <= stacks; ++i)
	{
		float stackAngle = glm::pi<float>() / 2 - i * stackStep;
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
			vertex.uv.s = (float)j / sectors;
			vertex.uv.t = (float)i / stacks;

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
