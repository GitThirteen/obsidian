module;
#include <native/macros.h>

// =========================== //
//    MAGMA - VECTOR MODULE    //
// =========================== //

export module Obsidian.Magma.Vector;

import std;
import Obsidian.Core.Concepts;

EXPORT(obsidian)

template<typename T, size_t N = 3>
requires IsNumeric<T>
struct Vector 
{
	using value_type = T;
	static constexpr size_t size = N;

    std::array<T, N> m_data;

    constexpr Vector() : m_data{0} {}

    template<typename... Args>
    requires (sizeof...(Args) == N) && (std::convertible_to<Args, T> && ...)
    constexpr Vector(Args&&... args) : m_data{ static_cast<T>(args)... } {}

    constexpr auto x() -> T& requires (N >= 1) { return m_data[0]; }
    constexpr auto x() const -> const T& requires (N >= 1) { return m_data[0]; }
    constexpr auto y() -> T& requires (N >= 2) { return m_data[1]; }
    constexpr auto y() const -> const T& requires (N >= 2) { return m_data[1]; }
    constexpr auto z() -> T& requires (N >= 3) { return m_data[2]; }
    constexpr auto z() const -> const T& requires (N >= 3) { return m_data[2]; }
    constexpr auto w() -> T& requires (N >= 4) { return m_data[3]; }
    constexpr auto w() const -> const T& requires (N >= 4) { return m_data[3]; }

    constexpr auto operator[](size_t i) -> T& { return m_data[i]; }
    constexpr auto operator[](size_t i) const -> const T& { return m_data[i]; }

    constexpr auto dot(const Vector& other) const -> T
    {
        T sum = 0;
        for (size_t i = 0; i < N; ++i)
        {
            sum += m_data[i] * other.m_data[i];
        }

        return sum;
    }

    constexpr auto cross(const Vector& other) const -> Vector requires (N == 3)
    {
        return Vector(
            y() * other.z() - z() * other.y(),
            z() * other.x() - x() * other.z(),
            x() * other.y() - y() * other.x()
        );
    }

    auto length() const -> T
    {
        return std::sqrt(dot(*this));
    }

    auto normalize() const -> Vector
    {
        T len = length();
        return (len == 0) ? *this : (*this / len);
    }

    static constexpr auto min(const Vector& a, const Vector& b) -> Vector
    {
        Vector result;
        for (size_t i = 0; i < N; ++i)
        {
            result.m_data[i] = std::min(a.m_data[i], b.m_data[i]);
        }

        return result;
    }

    static constexpr auto max(const Vector& a, const Vector& b) -> Vector
    {
        Vector result;
        for (size_t i = 0; i < N; ++i)
        {
            result.m_data[i] = std::max(a.m_data[i], b.m_data[i]);
        }
        
        return result;
    }

    // ========
    // Addition
    // ========

    constexpr auto operator+=(const Vector& o) -> Vector&
    {
        for (size_t i = 0; i < N; ++i)
        {
            m_data[i] += o.m_data[i];
        }

        return *this;
    }

    friend constexpr auto operator+(Vector l, const Vector& r) -> Vector
    {
        l += r; return l;
    }

    // ===========
    // Subtraction
    // ===========

    constexpr auto operator-=(const Vector& o) -> Vector&
    {
        for (size_t i = 0; i < N; ++i)
        {
            m_data[i] -= o.m_data[i];
        }

        return *this;
    }

    friend constexpr auto operator-(Vector l, const Vector& r) -> Vector
    {
        l -= r; return l;
    }

	// ==============
    // Multiplication
    // ==============

    constexpr auto operator*=(T scalar) -> Vector&
    {
        for (size_t i = 0; i < N; ++i)
        {
            m_data[i] *= scalar;
        }

        return *this;
    }

    constexpr auto operator*=(const Vector& o) -> Vector&
    {
        for (size_t i = 0; i < N; ++i)
        {
            m_data[i] *= o.m_data[i];
        }

        return *this;
    }

    friend constexpr auto operator*(Vector l, const Vector& r) -> Vector
    {
        l *= r; return l;
    }

    friend constexpr auto operator*(Vector l, T s) -> Vector
    {
        l *= s; return l;
    }

    friend constexpr auto operator*(T s, Vector r) -> Vector
    {
        r *= s; return r;
    }

    // ========
    // Division
    // ========

    constexpr auto operator/=(T scalar) -> Vector&
    {
        for (size_t i = 0; i < N; ++i)
        {
            m_data[i] /= scalar;
        }
        
        return *this;
    }

    constexpr auto operator/=(const Vector& o) -> Vector&
    {
        for (size_t i = 0; i < N; ++i)
        {
            m_data[i] /= o.m_data[i];
        }

        return *this;
    }

    friend constexpr auto operator/(Vector l, T s) -> Vector
    {
        l /= s; return l;
    }

    friend constexpr auto operator/(Vector l, const Vector& r) -> Vector
    {
        l /= r; return l;
    }

    friend constexpr auto operator/(T s, Vector r) -> Vector
    {
        for (size_t i = 0; i < N; ++i)
        {
            r.m_data[i] = s / r.m_data[i];
        }

        return r;
    }

    // =====
    // Other
    // =====

    constexpr auto operator-() const -> Vector
    {
        Vector result;
        for (size_t i = 0; i < N; ++i)
        {
            result.m_data[i] = -m_data[i];
        }

        return result;
    }

    friend constexpr bool operator==(const Vector&, const Vector&) = default;
};

EXPORT_END