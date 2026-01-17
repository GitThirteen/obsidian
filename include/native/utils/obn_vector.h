#pragma once

#ifndef OBSIDIAN_VECTOR
#define OBSIDIAN_VECTOR

#include <native/core/include.h>

template<typename T, size_t N = 3>
requires IsNumeric<T>
struct Vector
{
    Vector() : m_data{ 0 } { }

    template<typename... Args>
    requires (sizeof...(Args) == N) && (std::convertible_to<Args, T> && ...)
    Vector(Args&&... args) : m_data{ static_cast<T>(args)... } { }

    Vector(const Vector& other) = default;

    // Accessors

    constexpr auto x() -> T& requires (N >= 1) { return m_data[0]; }
    constexpr auto x() const -> const T& requires (N >= 1) { return m_data[0]; }

    constexpr auto y() -> T& requires (N >= 2) { return m_data[1]; }
    constexpr auto y() const -> const T& requires (N >= 2) { return m_data[1]; }

    constexpr auto z() -> T& requires (N >= 3) { return m_data[2]; }
    constexpr auto z() const -> const T& requires (N >= 3) { return m_data[2]; }

    constexpr auto w() -> T& requires (N >= 4) { return m_data[3]; }
    constexpr auto w() const -> const T& requires (N >= 4) { return m_data[3]; }

    constexpr T& operator[](size_t i) { assert(i < N); return m_data[i]; }
    constexpr const T& operator[](size_t i) const { assert(i < N); return m_data[i]; }

    // Algebra

    constexpr auto dot(const Vector& other) const -> T
    {
        T sum = 0;
        for (size_t i = 0; i < N; ++i)
        {
            sum += m_data[i] * other.m_data[i];
        }

        return sum;
    }

    constexpr auto wedge(const Vector& other) const -> T requires (N == 2)
    {
        return (x() * other.y()) - (y() * other.x());
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
        if (len == 0) return *this;
        return *this / len;
    }

    // Arithmetic Operators

    constexpr Vector& operator+=(const Vector& other)
    {
        for (size_t i = 0; i < N; ++i) m_data[i] += other.m_data[i]; return *this;
    }
    constexpr Vector& operator+=(T scalar)
    {
        for (size_t i = 0; i < N; ++i) m_data[i] += scalar; return *this;
    }

    constexpr Vector& operator-=(const Vector& other)
    {
        for (size_t i = 0; i < N; ++i) m_data[i] -= other.m_data[i]; return *this;
    }
    constexpr Vector& operator-=(T scalar)
    {
        for (size_t i = 0; i < N; ++i) m_data[i] -= scalar; return *this;
    }

    constexpr Vector& operator*=(const Vector& other)
    {
        for (size_t i = 0; i < N; ++i) m_data[i] -= other.m_data[i]; return *this;
    }
    constexpr Vector& operator*=(T scalar)
    {
        for (size_t i = 0; i < N; ++i) m_data[i] *= scalar; return *this;
    }

    constexpr Vector& operator/=(const Vector& other)
    {
        for (size_t i = 0; i < N; ++i) m_data[i] /= other.m_data[i]; return *this;
    }
    constexpr Vector& operator/=(T scalar)
    {
        for (size_t i = 0; i < N; ++i) m_data[i] /= scalar; return *this;
    }

    friend constexpr Vector operator+(Vector l, const Vector& r) { l += r; return l; }
    friend constexpr Vector operator+(Vector l, T s) { l += s; return l; }
    friend constexpr Vector operator-(Vector l, const Vector& r) { l -= r; return l; }
    friend constexpr Vector operator-(Vector l, T s) { l -= s; return l; }
    friend constexpr Vector operator*(Vector l, const Vector& r) { l *= r; return l; }
    friend constexpr Vector operator*(Vector l, T s) { l *= s; return l; }
    friend constexpr Vector operator/(Vector l, const Vector& r) { l /= r; return l; }
    friend constexpr Vector operator/(Vector l, T s) { l /= s; return l; }

    // Swizzling

    constexpr auto xy() const -> Vector<T, 2> requires (N >= 2)
    {
        return Vector<T, 2>(x(), y());
    }

    constexpr auto xz() const -> Vector<T, 2> requires (N >= 3)
    {
        return Vector<T, 2>(x(), z());
    }

    constexpr auto yz() const -> Vector<T, 2> requires (N >= 3)
    {
        return Vector<T, 2>(y(), z());
    }

    constexpr auto xyz() const -> Vector<T, 3> requires (N >= 3)
    {
        return Vector<T, 3>(x(), y(), z());
    }

    constexpr auto rgb() const -> Vector<T, 3> requires (N >= 3)
    {
        return xyz();
    }

    // Interop

    explicit operator std::vector<T>() const
    {
        return std::vector<T>(m_data.begin(), m_data.end());
    }

    explicit operator std::array<T, N>() const
    {
        return m_data;
    }

#ifdef GLM_VERSION
    explicit operator glm::vec<N, T>() const
    {
        glm::vec<N, T> ret;
        std::memcpy(&ret.x, m_data.data(), N * sizeof(T));
        return ret;
    }
#endif

#ifdef IMGUI_VERSION
    explicit operator ImVec2() const requires (N == 2)
    {
        return ImVec2(static_cast<float>(x()), static_cast<float>(y()));
    }

    explicit operator ImVec4() const requires (N == 4)
    {
        return ImVec4(static_cast<float>(x()), static_cast<float>(y()), static_cast<float>(z()), static_cast<float>(w()));
    }

    explicit operator ImVector<T>() const
    {
        ImVector<T> ret;
        for (const auto& v : m_data) ret.push_back(v);
        return ret;
    }
#endif

private:
    std::array<T, N> m_data;
};

#endif OBSIDIAN_VECTOR
