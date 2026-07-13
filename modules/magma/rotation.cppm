module;
#include <native/macros.h>

// ============================= //
//    MAGMA - ROTATION MODULE    //
// ============================= //

export module Obsidian.Magma:Rotation;
import std;
import :Vector;
import :Matrix;
import :Definitions;

EXPORT(obsidian)

struct EulerAngles;

struct alignas(16) Quaternion
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;

    constexpr Quaternion() = default;
    constexpr Quaternion(float qx, float qy, float qz, float qw) : x(qx), y(qy), z(qz), w(qw) { }

    static auto from_axis_angle(const Vector<float, 3>& axis, float angle) -> Quaternion
    {
        float half_angle = angle * 0.5f;
        float s = std::sin(half_angle);
        Vector<float, 3> norm_axis = axis.normalize();
        return Quaternion(norm_axis.x() * s, norm_axis.y() * s, norm_axis.z() * s, std::cos(half_angle));
    }

    static auto from_euler(const EulerAngles& euler) -> Quaternion;

    constexpr auto dot(const Quaternion& other) const -> float
    {
        return x * other.x + y * other.y + z * other.z + w * other.w;
    }

    auto length() const -> float
    {
        return std::sqrt(dot(*this));
    }

    auto normalize() const -> Quaternion
    {
        float len = length();
        if (len <= EPSILON)
        {
            return Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
        }

        float inv_len = 1.0f / len;
        return Quaternion(x * inv_len, y * inv_len, z * inv_len, w * inv_len);
    }

    constexpr auto conjugate() const -> Quaternion
    {
        return Quaternion(-x, -y, -z, w);
    }

    constexpr auto inverse() const -> Quaternion
    {
        float d = dot(*this);
        return Quaternion(-x / d, -y / d, -z / d, w / d);
    }

    friend constexpr auto operator*(const Quaternion& q1, const Quaternion& q2) -> Quaternion
    {
        return Quaternion(
            q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
            q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x,
            q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w,
            q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z
        );
    }

    constexpr auto operator*=(const Quaternion& other) -> Quaternion&
    {
        *this = *this * other;
        return *this;
    }

    friend constexpr auto operator*(const Quaternion& q, const Vector<float, 3>& v) -> Vector<float, 3>
    {
        // Quaternion-vector rotation: 
        // v' = v + 2w(q.xyz x v) + 2(q.xyz x (q.xyz x v))
        Vector<float, 3> q_vec(q.x, q.y, q.z);
        Vector<float, 3> t = 2.0f * q_vec.cross(v);
        return v + (q.w * t) + q_vec.cross(t);
    }

    static auto slerp(Quaternion q1, Quaternion q2, float t) -> Quaternion
    {
        q1 = q1.normalize();
        q2 = q2.normalize();

        float cos_theta = q1.dot(q2);

        // Take the shortest path across the sphere
        if (cos_theta < 0.0f)
        {
            q2 = Quaternion(-q2.x, -q2.y, -q2.z, -q2.w);
            cos_theta = -cos_theta;
        }

        if (cos_theta > 0.9995f)
        {
            return Quaternion(
                std::lerp(q1.x, q2.x, t),
                std::lerp(q1.y, q2.y, t),
                std::lerp(q1.z, q2.z, t),
                std::lerp(q1.w, q2.w, t)
            ).normalize();
        }

        float half_theta = std::acos(cos_theta);
        float sin_half_theta = std::sqrt(1.0f - cos_theta * cos_theta);

        float ratio_a = std::sin((1.0f - t) * half_theta) / sin_half_theta;
        float ratio_b = std::sin(t * half_theta) / sin_half_theta;

        return Quaternion(
            q1.x * ratio_a + q2.x * ratio_b,
            q1.y * ratio_a + q2.y * ratio_b,
            q1.z * ratio_a + q2.z * ratio_b,
            q1.w * ratio_a + q2.w * ratio_b
        );
    }

    constexpr auto to_matrix4x4() const -> Matrix4x4
    {
        Matrix4x4 res;
        float xx = x * x, yy = y * y, zz = z * z;
        float xy = x * y, xz = x * z, yz = y * z;
        float wx = w * x, wy = w * y, wz = w * z;

        res[0][0] = 1.0f - 2.0f * (yy + zz);
        res[0][1] = 2.0f * (xy + wz);
        res[0][2] = 2.0f * (xz - wy);
        res[0][3] = 0.0f;

        res[1][0] = 2.0f * (xy - wz);
        res[1][1] = 1.0f - 2.0f * (xx + zz);
        res[1][2] = 2.0f * (yz + wx);
        res[1][3] = 0.0f;

        res[2][0] = 2.0f * (xz + wy);
        res[2][1] = 2.0f * (yz - wx);
        res[2][2] = 1.0f - 2.0f * (xx + yy);
        res[2][3] = 0.0f;

        res[3][0] = 0.0f;
        res[3][1] = 0.0f;
        res[3][2] = 0.0f;
        res[3][3] = 1.0f;

        return res;
    }
};

struct EulerAngles
{
    float pitch = 0.0f;
    float yaw   = 0.0f;
    float roll  = 0.0f;

    constexpr EulerAngles() = default;
    constexpr EulerAngles(float p, float y, float r) : pitch(p), yaw(y), roll(r) {}

    static constexpr auto from_degrees(float p, float y, float r) -> EulerAngles
    {
        return EulerAngles(p * DEG_TO_RAD, y * DEG_TO_RAD, r * DEG_TO_RAD);
    }

    static auto from_quaternion(const Quaternion& q) -> EulerAngles
    {
        EulerAngles euler;

        // Roll (Z-axis)
        float sinr_cosp = 2.0f * (q.w * q.z + q.x * q.y);
        float cosr_cosp = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
        euler.roll = std::atan2(sinr_cosp, cosr_cosp);

        // Pitch (X-axis)
        float sinp = 2.0f * (q.w * q.x - q.z * q.y);
        if (std::abs(sinp) >= 1.0f)
        {
            euler.pitch = std::copysign(HALFPI, sinp); 
        }
        else
        {
            euler.pitch = std::asin(sinp);
        }

        // Yaw (Y-axis)
        float siny_cosp = 2.0f * (q.w * q.y + q.z * q.x);
        float cosy_cosp = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
        euler.yaw = std::atan2(siny_cosp, cosy_cosp);

        return euler;
    }
};

inline auto Quaternion::from_euler(const EulerAngles& euler) -> Quaternion
{
    float cy = std::cos(euler.yaw * 0.5f);
    float sy = std::sin(euler.yaw * 0.5f);
    float cp = std::cos(euler.pitch * 0.5f);
    float sp = std::sin(euler.pitch * 0.5f);
    float cr = std::cos(euler.roll * 0.5f);
    float sr = std::sin(euler.roll * 0.5f);

    return Quaternion(
        cy * sp * cr + sy * cp * sr, // x
        sy * cp * cr - cy * sp * sr, // y
        cy * cp * sr - sy * sp * cr, // z
        cy * cp * cr + sy * sp * sr  // w
    );
}

EXPORT_END