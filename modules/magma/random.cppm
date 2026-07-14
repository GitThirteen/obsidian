module;
#include <native/macros.h>

export module Obsidian.Magma.Random;

import std;
import Obsidian.Magma.Vector;
import Obsidian.Magma.Definitions;

EXPORT(obsidian, magma, random)

struct PCGEngine
{
    std::uint64_t state = 0x853c49e6748fea9bULL;
    std::uint64_t inc = 0xda3e39cb94b95bdbULL;

    constexpr PCGEngine() = default;
    constexpr PCGEngine(const std::uint64_t seed, const std::uint64_t seq = 0)
    {
        state = 0U; inc = (seq << 1u) | 1u;
        operator()(); state += seed; operator()();
    }

    constexpr auto operator()() -> std::uint32_t
    {
        std::uint64_t oldstate = state;
        state = oldstate * 6364136223846793005ULL + inc;
        std::uint32_t xorshifted = static_cast<std::uint32_t>(((oldstate >> 18u) ^ oldstate) >> 27u);
        const std::uint32_t rot = static_cast<std::uint32_t>(oldstate >> 59u);
        return (xorshifted >> rot) | (xorshifted << ((~rot + 1u) & 31));
    }
};

struct LCGEngine
{
    std::uint32_t state = 123456789;
    
    constexpr LCGEngine() = default;
    explicit constexpr LCGEngine(const std::uint32_t seed) : state(seed) {}

    constexpr auto operator()() -> std::uint32_t
    {
        state = state * 1664525 + 1013904223;
        return state;
    }
};

template<typename EngineType>
struct Generator
{
    EngineType engine;

    constexpr Generator() = default;
    
    template<typename... Args>
    explicit constexpr Generator(Args&&... args) : engine(std::forward<Args>(args)...) { }

    template <typename T>
    constexpr auto gen() -> T
    {
        if constexpr (std::is_floating_point_v<T>)
        {
            return static_cast<T>(engine()) * static_cast<T>(1.0 / 4294967296.0);
        }
        else if constexpr (std::is_integral_v<T>)
        {
            return static_cast<T>(engine());
        }
        else if constexpr (IsMagmaVector<T>)
        {
            T vec;
            for (size_t i = 0; i < T::size; ++i)
            {
                vec[i] = gen<typename T::value_type>();
            }
            return vec;
        }
        else
        {
            static_assert(sizeof(T) == 0, "Type not supported by Random::gen<T>()");
        }
    }

    template <typename T>
    constexpr auto between(T min, T max) -> T
    {
        static_assert(std::is_floating_point_v<T>, "between() is strictly for floating-point types. Use roll() for integers.");
        return min + gen<T>() * (max - min);
    }

    template <typename T>
    constexpr auto roll(T min, T max) -> T
    {
        static_assert(std::is_integral_v<T>, "roll() is strictly for integral types. Use between() for floating-point values.");
        std::uint32_t bound = static_cast<std::uint32_t>(max - min + 1);
        return min + static_cast<T>(engine() % bound);
    }

    auto gen_unit_vec() -> Vector<float, 3>
    {
        while (true)
        {
            Vector<float, 3> p(between(-1.0f, 1.0f), between(-1.0f, 1.0f), between(-1.0f, 1.0f));
            float sq_length = p.dot(p);

            if (sq_length > EPSILON && sq_length <= 1.0f)
            {
                return p / std::sqrt(sq_length);
            }
        }
    }
};

using PCG      = Generator<PCGEngine>;
using LCG      = Generator<LCGEngine>;
using Mersenne = Generator<std::mt19937_64>;

inline auto global() -> PCG&
{
    thread_local PCG instance{ std::random_device{}() };
    return instance;
}

template <typename T>
inline auto gen() -> T 
{ 
    return global().gen<T>(); 
}

template <typename T>
inline auto between(T min, T max) -> T 
{ 
    return global().between<T>(min, max); 
}

template <typename T>
inline auto roll(T min, T max) -> T 
{ 
    return global().roll<T>(min, max); 
}

inline auto gen_unit_vec() -> Vector<float, 3> 
{ 
    return global().gen_unit_vec(); 
}

EXPORT_END