#pragma once

#include <native/core/obsidian/include.h>

struct Variadic
{
    template <typename Vec, typename F, size_t MaxSize = 6>
    static auto apply_vector(Vec& vec, F&& f) {
        if (vec.empty()) {
            return std::forward<F>(f)();
        }

        if (vec.size() > MaxSize) {
            throw std::runtime_error("Vector too large for variadic apply.");
        }

        return dispatch_vector<Vec, F, MaxSize>(vec, std::forward<F>(f));
    }

private:
    template <typename Vec, typename F, size_t... Is>
    static auto call_with_indices(Vec& vec, F&& f, std::index_sequence<Is...>) {
        return f(vec[Is]...);
    }

    template <typename Vec, typename F, size_t N>
    static auto dispatch_vector(Vec& vec, F&& f) {
        if (vec.size() == N) {
            return call_with_indices(vec, std::forward<F>(f), std::make_index_sequence<N>{});
        }

        if constexpr (N > 1) {
            return dispatch_vector<Vec, F, (N - 1)>(vec, std::forward<F>(f));
        }
    }
};