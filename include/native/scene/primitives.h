#pragma once

#include <native/core/include.h>
#include <native/scene/object.h>

enum class GeometricPrimitiveType
{
	Plane,
	Cube,
	Sphere
};

struct Primitives
{
	static std::shared_ptr<SceneObject> generate(Root& root, GeometricPrimitiveType type);

	static GeometryData _generate_plane_gm();
	static GeometryData _generate_cube_gm();
	static GeometryData _generate_sphere_gm(int sectors, int stacks);

private:
	inline static std::array<uint32_t, 3> internal_counter = { };
};