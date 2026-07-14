module;
#include <native/macros.h>

// ============================= //
//      MAGMA - WARP MODULE      //
// ============================= //

export module Obsidian.Magma.Warp;

import std;
import Obsidian.Magma.Vector;
import Obsidian.Magma.Definitions;
import Obsidian.Magma.Random;

EXPORT(obsidian, magma, warp)

constexpr auto to_tent(const Vector<float, 2>& sample) -> Vector<float, 2>
{
    Vector<float, 2> val(0.0f, 0.0f);
    val.x() = (sample.x() < 0.5f) ? std::sqrt(2.0f * sample.x()) - 1.0f : 1.0f - std::sqrt(2.0f - 2.0f * sample.x());
    val.y() = (sample.y() < 0.5f) ? std::sqrt(2.0f * sample.y()) - 1.0f : 1.0f - std::sqrt(2.0f - 2.0f * sample.y());
    return val;
}

constexpr auto to_concentric_disk(const Vector<float, 2>& u) -> Vector<float, 2>
{
    // Peter Shirley's method
    Vector<float, 2> u_offset = u * 2.0f - Vector<float, 2>(1.0f, 1.0f);
    if (u_offset.x() == 0.0f && u_offset.y() == 0.0f) return Vector<float, 2>(0.0f, 0.0f);

    float theta, r;
    if (std::abs(u_offset.x()) > std::abs(u_offset.y()))
    {
        r = u_offset.x();
        theta = (HALFPI * 0.5f) * (u_offset.y() / u_offset.x());
    }
    else
    {
        r = u_offset.y();
        theta = HALFPI - (HALFPI * 0.5f) * (u_offset.x() / u_offset.y());
    }
    return Vector<float, 2>(r * std::cos(theta), r * std::sin(theta));
}

constexpr auto to_uniform_sphere(const Vector<float, 2>& u) -> Vector<float, 3>
{
    float z = 1.0f - 2.0f * u.x();
    float r = std::sqrt(std::max(0.0f, 1.0f - z * z));
    float phi = TWOPI * u.y();
    return Vector<float, 3>(r * std::cos(phi), r * std::sin(phi), z);
}

constexpr auto to_uniform_hemisphere(const Vector<float, 2>& u) -> Vector<float, 3>
{
    float z = u.x();
    float r = std::sqrt(std::max(0.0f, 1.0f - z * z));
    float phi = TWOPI * u.y();
    return Vector<float, 3>(r * std::cos(phi), r * std::sin(phi), z);
}

constexpr auto to_cosine_hemisphere(const Vector<float, 2>& u) -> Vector<float, 3>
{
    // Malley's Method
    Vector<float, 2> d = to_concentric_disk(u);
    float z = std::sqrt(std::max(0.0f, 1.0f - d.x() * d.x() - d.y() * d.y()));
    return Vector<float, 3>(d.x(), d.y(), z);
}

inline auto to_beckmann(const Vector<float, 2>& sample, float alpha) -> Vector<float, 3>
{
    float log_sample = std::log(1.0f - sample.y());
    if (std::isinf(log_sample)) log_sample = 0.0f;

    float theta = sample.x() * TWOPI;
    float phi = std::atan(std::sqrt(-alpha * alpha * log_sample));

    return Vector<float, 3>(
        std::sin(phi) * std::cos(theta),
        std::sin(phi) * std::sin(theta),
        std::cos(phi)
    );
}

// GGX (Trowbridge-Reitz) distribution
inline auto to_ggx(const Vector<float, 2>& sample, const float alpha) -> Vector<float, 3>
{
    float a2 = alpha * alpha;
    float cos_theta = std::sqrt((1.0f - sample.y()) / (1.0f + (a2 - 1.0f) * sample.y()));
    float sin_theta = std::sqrt(std::max(0.0f, 1.0f - cos_theta * cos_theta));
    float phi = TWOPI * sample.x();

    return Vector<float, 3>(
        sin_theta * std::cos(phi),
        sin_theta * std::sin(phi),
        cos_theta
    );
}

// GGX-VNDF
constexpr auto to_ggx_vndf(const Vector<float, 2>& u, const Vector<float, 3>& V, float alpha) -> Vector<float, 3>
{
    Vector<float, 3> vh(alpha * V.x(), alpha * V.y(), V.z());
    vh = vh.normalize();

    Vector<float, 3> t1 = (vh.z() < 0.9999f) ? Vector<float, 3>(0.0f, 0.0f, 1.0f).cross(vh).normalize() : Vector<float, 3>(1.0f, 0.0f, 0.0f);
    Vector<float, 3> t2 = vh.cross(t1);

    float r = std::sqrt(u.x());
    float phi = TWOPI * u.y();
    float t1_val = r * std::cos(phi);
    float t2_val = r * std::sin(phi);
    float s = 0.5f * (1.0f + vh.z());
    t2_val = (1.0f - s) * std::sqrt(1.0f - t1_val * t1_val) + s * t2_val;

    Vector<float, 3> nh = t1 * t1_val + t2 * t2_val + vh * std::sqrt(std::max(0.0f, 1.0f - t1_val * t1_val - t2_val * t2_val));

    const Vector<float, 3> h(alpha * nh.x(), alpha * nh.y(), std::max(0.0f, nh.z()));
    return h.normalize();
}

EXPORT_END

EXPORT(obsidian, magma, pdf)

constexpr auto tent(const Vector<float, 2>& p) -> float
{
    float x = (p.x() < -1.0f || p.x() > 1.0f) ? 0.0f : 1.0f - std::abs(p.x());
    float y = (p.y() < -1.0f || p.y() > 1.0f) ? 0.0f : 1.0f - std::abs(p.y());
    return x * y;
}

constexpr auto concentric_disk(const Vector<float, 2>& p) -> float
{
    if (p.length() < 1.0f) return INVPI;
    return 0.0f;
}

constexpr auto uniform_sphere() -> float
{
    return 0.25f * INVPI;
}

constexpr auto uniform_hemisphere(const Vector<float, 3>& v) -> float
{
    if (v.z() < 0.0f) return 0.0f;
    return INVPI * 0.5f;
}

constexpr auto cosine_hemisphere(const Vector<float, 3>& v) -> float
{
    if (v.z() < 0.0f) return 0.0f;
    return v.z() * INVPI;
}

constexpr auto beckmann(const Vector<float, 3>& m, const float alpha) -> float
{
    if (m.z() <= 0.0f) return 0.0f;

    const float tan2phi = (m.x() * m.x() + m.y() * m.y()) / (m.z() * m.z());
    const float cos3phi = m.z() * m.z() * m.z();

    constexpr float azimuthal = INVPI;
    const float upper = std::exp(-tan2phi / (alpha * alpha));
    const float lower = (alpha * alpha) * cos3phi;
    const float longitudinal = upper / lower;
    
    return azimuthal * longitudinal;
}

constexpr auto ggx(const Vector<float, 3>& m, const float alpha) -> float
{
    if (m.z() <= 0.0f) return 0.0f;

    const float a2 = alpha * alpha;
    const float cos_theta = m.z();
    const float cos2_theta = cos_theta * cos_theta;
    
    const float denom = cos2_theta * (a2 - 1.0f) + 1.0f;
    const float d_m = (a2 * INVPI) / (denom * denom);
    
    return d_m * cos_theta; 
}

constexpr auto ggx_vndf(const Vector<float, 3>& V, const Vector<float, 3>& H, float alpha) -> float
{
    if (H.z() <= 0.0f) return 0.0f;
    
    const float v_dot_h = std::max(0.0f, V.dot(H));
    if (v_dot_h == 0.0f) return 0.0f;

    const float alpha2 = alpha * alpha;
    
    const float cos_theta = H.z();
    const float cos2_theta = cos_theta * cos_theta;
    const float denom_d = cos2_theta * (alpha2 - 1.0f) + 1.0f;
    const float d_m = (alpha2 * INVPI) / (denom_d * denom_d);

    const float cos_theta_v = std::max(V.z(), EPSILON);
    const float cos2_theta_v = cos_theta_v * cos_theta_v;
    const float tan2_theta_v = std::max(0.0f, (1.0f - cos2_theta_v) / cos2_theta_v);
    const float lambda_v = 0.5f * (std::sqrt(1.0f + alpha2 * tan2_theta_v) - 1.0f);
    const float g1 = 1.0f / (1.0f + lambda_v);

    return (g1 * v_dot_h * d_m) / cos_theta_v;
}

EXPORT_END