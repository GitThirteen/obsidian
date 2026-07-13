module;
#include <native/macros.h>

// =========================== //
//   MAGMA - GEOMETRY MODULE   //
// =========================== //

export module Obsidian.Magma:Geometry;
import std;
import :Vector;
import :Matrix;
import :Definitions;

EXPORT(obsidian)

struct AABB
{
    Vector<float> min;
    Vector<float> max;

    constexpr AABB() = default;
    constexpr AABB(const Vector<float>& min_point, const Vector<float>& max_point) : min(min_point), max(max_point) { }

    [[nodiscard]] auto center() const -> Vector<float>
    {
        return (min + max) * 0.5f;
    }

    [[nodiscard]] auto size() const -> Vector<float>
    {
        return max - min;
    }

    auto transform(const Matrix4x4& matrix) const -> AABB
    {
        Vector<float> corners[8] = {
            { min.x(), min.y(), min.z() },
            { max.x(), min.y(), min.z() },
            { min.x(), max.y(), min.z() },
            { max.x(), max.y(), min.z() },
            { min.x(), min.y(), max.z() },
            { max.x(), min.y(), max.z() },
            { min.x(), max.y(), max.z() },
            { max.x(), max.y(), max.z() }
        };

        auto transform_point = [&](const Vector<float>& v) -> Vector<float> {
            auto vec4 = matrix * Vector<float, 4>(v.x(), v.y(), v.z(), 1.0f);
            return Vector<float>(vec4.x(), vec4.y(), vec4.z());
        };

        Vector<float> new_min = transform_point(corners[0]);
        Vector<float> new_max = new_min;

        for (int i = 1; i < 8; ++i)
        {
            Vector<float> transformed_corner = transform_point(corners[i]);
            new_min = Vector<float>::min(new_min, transformed_corner);
            new_max = Vector<float>::max(new_max, transformed_corner);
        }

        return AABB(new_min, new_max);
    }

    auto merge(const AABB& other) const -> AABB
    {
        Vector<float> new_min = Vector<float>::min(min, other.min);
        Vector<float> new_max = Vector<float>::max(max, other.max);

        return AABB(new_min, new_max);
    }

    auto expand(Vector<float> point) const -> AABB
    {
        Vector<float> new_min = Vector<float>::min(min, point);
        Vector<float> new_max = Vector<float>::max(max, point);

        return AABB(new_min, new_max);
    }

    // TODO: Implement intersection tests with rays, spheres, and other AABBs
};

struct Sphere
{
    Vector<float> center;
    float radius;

    constexpr Sphere() : center(0.0f, 0.0f, 0.0f), radius(1.0f) { }
    constexpr Sphere(const Vector<float>& c, float r) : center(c), radius(r) { }
};

struct Plane
{
    Vector<float> normal;
    float distance;

    constexpr Plane() : normal(0.0f, 1.0f, 0.0f), distance(0.0f) { }
    constexpr Plane(const Vector<float>& n, float d) : normal(n), distance(d) { }
};

struct Ray
{
    Vector<float> origin;
    Vector<float> direction;
    float t_min = EPSILON;
    float t_max = std::numeric_limits<float>::infinity();

    constexpr Ray() : origin(0.0f, 0.0f, 0.0f), direction(0.0f, 0.0f, -1.0f) { }
    constexpr Ray(const Vector<float>& o, const Vector<float>& d) : origin(o), direction(d) { }
    constexpr Ray(const Vector<float>& o, const Vector<float>& d, float tmin, float tmax) : origin(o), direction(d), t_min(tmin), t_max(tmax) { }

    // TODO: Implement ray intersection tests with planes, aabbs, spheres, etc.
};

struct Frustum
{
    Plane left;
    Plane right;
    Plane top;
    Plane bottom;
    Plane near;
    Plane far;

    constexpr Frustum() = default;
    constexpr Frustum(const Plane& l, const Plane& r, const Plane& t, const Plane& b, const Plane& n, const Plane& f)
        : left(l), right(r), top(t), bottom(b), near(n), far(f) { }

    // TODO: Extract frustum
    // TODO: Check if frustum intersects sphere
    // TODO: Check if frustum intersects AABB
};

EXPORT_END