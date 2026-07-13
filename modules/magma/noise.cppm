module;
#include <native/macros.h>

// ======================== //
//   MAGMA - NOISE MODULE   //
// ======================== //

export module Obsidian.Magma:Noise;
import std;
import :Vector;

EXPORT(obsidian, noise)

constexpr auto hash(std::int32_t x, std::int32_t y, std::uint32_t seed = 0) -> std::uint32_t
{
    std::uint32_t h = static_cast<std::uint32_t>(x) * 374761393U + static_cast<std::uint32_t>(y) * 668265263U + seed;
    h = (h ^ (h >> 13)) * 1274126177U;
    return h ^ (h >> 16);
}

constexpr auto gradient2D(std::int32_t x, std::int32_t y, std::uint32_t seed) -> Vector<float, 2>
{
    switch (hash(x, y, seed) & 3)
    {
        case 0:     return Vector<float, 2>( 1.0f,  1.0f);
        case 1:     return Vector<float, 2>(-1.0f,  1.0f);
        case 2:     return Vector<float, 2>( 1.0f, -1.0f);
        case 3:     return Vector<float, 2>(-1.0f, -1.0f);
        default:    return Vector<float, 2>( 0.0f,  0.0f);
    }
}

constexpr auto fade(float t) -> float
{
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

auto perlin2D(const Vector<float, 2>& position, std::uint32_t seed = 0) -> float
{
    // Find grid cell coordinates (top left corner)
    std::int32_t x0 = static_cast<std::int32_t>(std::floor(position.x()));
    std::int32_t y0 = static_cast<std::int32_t>(std::floor(position.y()));
    std::int32_t x1 = x0 + 1;
    std::int32_t y1 = y0 + 1;

    // Relative coordinates inside the grid cell (0.0 to 1.0)
    float sx = position.x() - static_cast<float>(x0);
    float sy = position.y() - static_cast<float>(y0);

    // Calculate smooth fade curves for interpolation
    float u = fade(sx);
    float v = fade(sy);

    // Distance vectors from the four cell corners to the point
    Vector<float, 2> d00(sx, sy);
    Vector<float, 2> d10(sx - 1.0f, sy);
    Vector<float, 2> d01(sx, sy - 1.0f);
    Vector<float, 2> d11(sx - 1.0f, sy - 1.0f);

    // Dot products between the random gradients and the distance vectors
    float dot00 = gradient2D(x0, y0, seed).dot(d00);
    float dot10 = gradient2D(x1, y0, seed).dot(d10);
    float dot01 = gradient2D(x0, y1, seed).dot(d01);
    float dot11 = gradient2D(x1, y1, seed).dot(d11);

    // Bilinear interpolation using the fade curves
    float lerpX1 = std::lerp(dot00, dot10, u);
    float lerpX2 = std::lerp(dot01, dot11, u);

    return std::lerp(lerpX1, lerpX2, v);
}

auto fBm2D(Vector<float, 2> position, int octaves, float persistence = 0.5f, float lacunarity = 2.0f, std::uint32_t seed = 0) -> float
{
    float total = 0.0f;
    float amplitude = 1.0f;
    float max_value = 0.0f;

    for (int i = 0; i < octaves; ++i)
    {
        total += perlin2D(position, seed) * amplitude;
        max_value += amplitude;
        
        amplitude *= persistence;
        position *= lacunarity;
        seed++;
    }

    return total / max_value;
}

// TODO: Simplex Noise 2D/3D
// TODO: Worley/Cellular Noise
// TODO: 3D Hash and 3D Perlin Noise (volumetric fog & 3D worldgen)

EXPORT_END