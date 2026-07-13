module;
#include <native/macros.h>

// =========================== //
//   MAGMA - SAMPLING MODULE   //
// =========================== //

export module Obsidian.Magma:Sampler;
import std;
import :Vector;
import :Random;

EXPORT(obsidian, magma, sampler)

struct Independent
{
    constexpr Independent() = default;

    auto next_1d() -> float
    {
        return random::gen<float>();
    }

    auto next_2d() -> Vector<float, 2>
    {
        return random::gen<Vector<float, 2>>();
    }
};

struct Stratified
{
    int x_grid;
    int y_grid;
    int curr_sample;

    constexpr Stratified(int x, int y) : x_grid(x), y_grid(y), curr_sample(0) {}

    auto next_2d() -> Vector<float, 2>
    {
        if (curr_sample >= x_grid * y_grid)
        {
            curr_sample++;
            return Vector<float, 2>(0.0f, 0.0f);
        }

        int x = curr_sample % x_grid;
        int y = curr_sample / x_grid;

        float dx = random::gen<float>();
        float dy = random::gen<float>();

        float u = (x + dx) / static_cast<float>(x_grid);
        float v = (y + dy) / static_cast<float>(y_grid);

        curr_sample++;
        return Vector<float, 2>(u, v);
    }
};

struct Halton
{
    std::uint32_t curr_index = 1;

    constexpr Halton() = default;

    static constexpr auto radical_inv(std::uint32_t n, std::uint32_t base) -> float
    {
        float value = 0.0f;
        float inv_base = 1.0f / static_cast<float>(base);
        float inv_bi = inv_base;

        while (n > 0)
        {
            std::uint32_t digit = n % base;
            value += digit * inv_bi;
            n /= base;
            inv_bi *= inv_base;
        }

        return value;
    }

    auto next_2d() -> Vector<float, 2>
    {
        float u = radical_inv(curr_index, 2);
        float v = radical_inv(curr_index, 3);
        curr_index++;

        return Vector<float, 2>(u, v);
    }
};

EXPORT_END