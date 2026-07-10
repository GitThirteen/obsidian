export module Obsidian.Utils:Magma;
import std;

export namespace obsidian 
{
    struct Variadic 
    {
        template <typename Vec, typename F, size_t MaxSize = 6>
        static auto apply_vector(Vec& vec, F&& f) -> decltype(auto)
        {
            if (vec.empty()) return std::forward<F>(f)();
            if (vec.size() > MaxSize) throw std::runtime_error("Vector too large for variadic apply.");
            return dispatch_vector<Vec, F, MaxSize>(vec, std::forward<F>(f));
        }

    private:
        template <typename Vec, typename F, size_t... Is>
        static auto call_with_indices(Vec& vec, F&& f, std::index_sequence<Is...>) -> decltype(auto)
        {
            return f(vec[Is]...);
        }

        template <typename Vec, typename F, size_t N>
        static auto dispatch_vector(Vec& vec, F&& f) -> decltype(auto)
        {
            if (vec.size() == N) return call_with_indices(vec, std::forward<F>(f), std::make_index_sequence<N>{});
            if constexpr (N > 1) return dispatch_vector<Vec, F, (N - 1)>(vec, std::forward<F>(f));
        }
    };

    // --- VECTOR MATH ---
    template<typename T>
    concept IsNumeric = std::is_arithmetic_v<T>;

    template<typename T, size_t N = 3>
    requires IsNumeric<T>
    struct Vector 
    {
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
            for (size_t i = 0; i < N; ++i) sum += m_data[i] * other.m_data[i];
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

        constexpr auto operator+=(const Vector& o) -> Vector&
        {
            for (size_t i = 0; i < N; ++i) m_data[i] += o.m_data[i]; return *this;
        }

        constexpr auto operator/=(T scalar) -> Vector&
        {
            for (size_t i=0; i<N; ++i) m_data[i] /= scalar; return *this;
        }
        
        friend constexpr auto operator+(Vector l, const Vector& r) -> Vector
        {
            l += r; return l;
        }

        friend constexpr auto operator/(Vector l, T s) -> Vector
        {
            l /= s; return l;
        }
    };
}