module;
#include <native/macros.h>

// ============================= //
//     MAGMA - MATRIX MODULE     //
// ============================= //

export module Obsidian.Magma:Matrix;
import std;
import :Vector;

EXPORT(obsidian)

struct alignas(16) Matrix4x4
{
    Vector<float, 4> columns[4];

    constexpr Matrix4x4()
    {
        columns[0] = Vector<float, 4>(1.0f, 0.0f, 0.0f, 0.0f);
        columns[1] = Vector<float, 4>(0.0f, 1.0f, 0.0f, 0.0f);
        columns[2] = Vector<float, 4>(0.0f, 0.0f, 1.0f, 0.0f);
        columns[3] = Vector<float, 4>(0.0f, 0.0f, 0.0f, 1.0f);
    }

    constexpr Matrix4x4(const Vector<float, 4>& c0, const Vector<float, 4>& c1, 
                        const Vector<float, 4>& c2, const Vector<float, 4>& c3)
    {
        columns[0] = c0;
        columns[1] = c1;
        columns[2] = c2;
        columns[3] = c3;
    }

    constexpr auto operator[](size_t i) -> Vector<float, 4>& { return columns[i]; }
    constexpr auto operator[](size_t i) const -> const Vector<float, 4>& { return columns[i]; }

    static constexpr auto identity() -> Matrix4x4
    {
        return Matrix4x4();
    }

    constexpr auto transpose() const -> Matrix4x4
    {
        return Matrix4x4(
            Vector<float, 4>(columns[0].x(), columns[1].x(), columns[2].x(), columns[3].x()),
            Vector<float, 4>(columns[0].y(), columns[1].y(), columns[2].y(), columns[3].y()),
            Vector<float, 4>(columns[0].z(), columns[1].z(), columns[2].z(), columns[3].z()),
            Vector<float, 4>(columns[0].w(), columns[1].w(), columns[2].w(), columns[3].w())
        );
    }

    constexpr auto inverse() const -> Matrix4x4
    {
        const float c00 = columns[2].z() * columns[3].w() - columns[3].z() * columns[2].w();
        const float c02 = columns[1].z() * columns[3].w() - columns[3].z() * columns[1].w();
        const float c03 = columns[1].z() * columns[2].w() - columns[2].z() * columns[1].w();
        const float c04 = columns[2].y() * columns[3].w() - columns[3].y() * columns[2].w();
        const float c06 = columns[1].y() * columns[3].w() - columns[3].y() * columns[1].w();
        const float c07 = columns[1].y() * columns[2].w() - columns[2].y() * columns[1].w();
        const float c08 = columns[2].y() * columns[3].z() - columns[3].y() * columns[2].z();
        const float c10 = columns[1].y() * columns[3].z() - columns[3].y() * columns[1].z();
        const float c11 = columns[1].y() * columns[2].z() - columns[2].y() * columns[1].z();
        const float c12 = columns[2].x() * columns[3].w() - columns[3].x() * columns[2].w();
        const float c14 = columns[1].x() * columns[3].w() - columns[3].x() * columns[1].w();
        const float c15 = columns[1].x() * columns[2].w() - columns[2].x() * columns[1].w();
        const float c16 = columns[2].x() * columns[3].z() - columns[3].x() * columns[2].z();
        const float c18 = columns[1].x() * columns[3].z() - columns[3].x() * columns[1].z();
        const float c19 = columns[1].x() * columns[2].z() - columns[2].x() * columns[1].z();
        const float c20 = columns[2].x() * columns[3].y() - columns[3].x() * columns[2].y();
        const float c22 = columns[1].x() * columns[3].y() - columns[3].x() * columns[1].y();
        const float c23 = columns[1].x() * columns[2].y() - columns[2].x() * columns[1].y();

        const float fac00 = columns[1].y() * c00 - columns[1].z() * c04 + columns[1].w() * c08;
        const float fac01 = columns[1].x() * c00 - columns[1].z() * c12 + columns[1].w() * c16;
        const float fac02 = columns[1].x() * c04 - columns[1].y() * c12 + columns[1].w() * c20;
        const float fac03 = columns[1].x() * c08 - columns[1].y() * c16 + columns[1].z() * c20;

        const float fac04 = columns[0].y() * c00 - columns[0].z() * c04 + columns[0].w() * c08;
        const float fac05 = columns[0].x() * c00 - columns[0].z() * c12 + columns[0].w() * c16;
        const float fac06 = columns[0].x() * c04 - columns[0].y() * c12 + columns[0].w() * c20;
        const float fac07 = columns[0].x() * c08 - columns[0].y() * c16 + columns[0].z() * c20;

        const float fac08 = columns[0].y() * c02 - columns[0].z() * c06 + columns[0].w() * c10;
        const float fac09 = columns[0].x() * c02 - columns[0].z() * c14 + columns[0].w() * c18;
        const float fac10 = columns[0].x() * c06 - columns[0].y() * c14 + columns[0].w() * c22;
        const float fac11 = columns[0].x() * c10 - columns[0].y() * c18 + columns[0].z() * c22;

        const float fac12 = columns[0].y() * c03 - columns[0].z() * c07 + columns[0].w() * c11;
        const float fac13 = columns[0].x() * c03 - columns[0].z() * c15 + columns[0].w() * c19;
        const float fac14 = columns[0].x() * c07 - columns[0].y() * c15 + columns[0].w() * c23;
        const float fac15 = columns[0].x() * c11 - columns[0].y() * c19 + columns[0].z() * c23;

        const float det = columns[0].x() * fac00 - columns[0].y() * fac01 + columns[0].z() * fac02 - columns[0].w() * fac03;
        const float invDet = 1.0f / det;

        return Matrix4x4(
            Vector<float, 4>( fac00 * invDet, -fac04 * invDet,  fac08 * invDet, -fac12 * invDet),
            Vector<float, 4>(-fac01 * invDet,  fac05 * invDet, -fac09 * invDet,  fac13 * invDet),
            Vector<float, 4>( fac02 * invDet, -fac06 * invDet,  fac10 * invDet, -fac14 * invDet),
            Vector<float, 4>(-fac03 * invDet,  fac07 * invDet, -fac11 * invDet,  fac15 * invDet)
        );
    }

    static auto perspective(const float fov_y, const float aspect, const float near_plane) -> Matrix4x4
    {
        const float f = 1.0f / std::tan(fov_y * 0.5f);
        Matrix4x4 result;

        result[0][0] = f / aspect;
        result[1][1] = -f; // Vulkan Y-down flip
        result[2][2] = 0.0f; // Far plane at infinity
        result[2][3] = -1.0f;
        result[3][2] = near_plane;
        result[3][3] = 0.0f;

        return result;
    }

    static auto perspective(const float fov_y, const float aspect, const float near_plane, const float far_plane) -> Matrix4x4
    {
        float f = 1.0f / std::tan(fov_y * 0.5f);
        Matrix4x4 result;
        
        result[0][0] = f / aspect;
        result[1][1] = -f; // Vulkan Y-down flip
        result[2][2] = near_plane / (far_plane - near_plane);
        result[2][3] = -1.0f; // Right-handed, looking down -Z
        result[3][2] = (far_plane * near_plane) / (far_plane - near_plane);
        result[3][3] = 0.0f;
        
        return result;
    }

    static auto orthographic(const float left, const float right, const float bottom, const float top, const float near_plane, float far_plane) -> Matrix4x4
    {
        Matrix4x4 result;
        
        result[0][0] = 2.0f / (right - left);
        result[1][1] = -2.0f / (top - bottom); // Vulkan Y-down flip
        result[2][2] = 1.0f / (far_plane - near_plane); // Reversed Z bounds
        
        result[3][0] = -(right + left) / (right - left);
        result[3][1] = -(top + bottom) / (top - bottom);
        result[3][2] = far_plane / (far_plane - near_plane); // Reversed Z bounds
        result[3][3] = 1.0f;

        return result;
    }

    static auto look_at(const Vector<float, 3>& eye, const Vector<float, 3>& target, const Vector<float, 3>& up) -> Matrix4x4
    {
        Vector<float, 3> f = (target + (eye * -1.0f)).normalize(); 
        Vector<float, 3> s = f.cross(up).normalize();
        Vector<float, 3> u = s.cross(f);

        Matrix4x4 result;
        result[0][0] = s.x();
        result[1][0] = s.y();
        result[2][0] = s.z();
        
        result[0][1] = u.x();
        result[1][1] = u.y();
        result[2][1] = u.z();
        
        result[0][2] = -f.x();
        result[1][2] = -f.y();
        result[2][2] = -f.z();

        result[3][0] = -s.dot(eye);
        result[3][1] = -u.dot(eye);
        result[3][2] = f.dot(eye);

        return result;
    }

    friend constexpr auto operator*(const Matrix4x4& a, const Matrix4x4& b) -> Matrix4x4
    {
        Matrix4x4 res;

        for(size_t c = 0; c < 4; ++c) 
        {
            for(size_t r = 0; r < 4; ++r) 
            {
                res[c][r] = a[0][r] * b[c][0] + 
                            a[1][r] * b[c][1] + 
                            a[2][r] * b[c][2] + 
                            a[3][r] * b[c][3];
            }
        }

        return res;
    }

    friend constexpr auto operator*(const Matrix4x4& m, const Vector<float, 4>& v) -> Vector<float, 4>
    {
        return Vector<float, 4>(
            m[0].x() * v.x() + m[1].x() * v.y() + m[2].x() * v.z() + m[3].x() * v.w(),
            m[0].y() * v.x() + m[1].y() * v.y() + m[2].y() * v.z() + m[3].y() * v.w(),
            m[0].z() * v.x() + m[1].z() * v.y() + m[2].z() * v.z() + m[3].z() * v.w(),
            m[0].w() * v.x() + m[1].w() * v.y() + m[2].w() * v.z() + m[3].w() * v.w()
        );
    }
};

struct Matrix3x3
{
    Vector<float, 3> columns[3];

    constexpr Matrix3x3()
    {
        columns[0] = Vector<float, 3>(1.0f, 0.0f, 0.0f);
        columns[1] = Vector<float, 3>(0.0f, 1.0f, 0.0f);
        columns[2] = Vector<float, 3>(0.0f, 0.0f, 1.0f);
    }

    constexpr auto operator[](size_t i) -> Vector<float, 3>& { return columns[i]; }
    constexpr auto operator[](size_t i) const -> const Vector<float, 3>& { return columns[i]; }

    constexpr auto transpose() const -> Matrix3x3
    {
        Matrix3x3 result;
        result[0] = Vector<float, 3>(columns[0].x(), columns[1].x(), columns[2].x());
        result[1] = Vector<float, 3>(columns[0].y(), columns[1].y(), columns[2].y());
        result[2] = Vector<float, 3>(columns[0].z(), columns[1].z(), columns[2].z());
        
        return result;
    }
};

EXPORT_END