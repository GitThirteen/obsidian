module;
#include <native/macros.h>

// =========================== //
//    CORE - RESULT MODULE     //
// =========================== //

export module Obsidian.Core.Result;
import std;
import Obsidian.Core.Logger;

EXPORT(obsidian)

template <typename T, typename E = std::string>
struct Result 
{
    std::expected<T, E> inner;

    Result(std::expected<T, E> e) : inner(std::move(e)) { }
    
    Result(std::unexpected<E> err) : inner(std::move(err)) { }

    Result() requires std::is_void_v<T> : inner() { }

    template <typename U = T> requires (!std::is_void_v<U> && std::convertible_to<U, T>)
    Result(U&& val) : inner(std::forward<U>(val)) { }

    explicit operator bool() const
    { 
        return inner.has_value();
    }

    auto error() const -> const E&
    { 
        return inner.error();
    }

    auto expect(std::string_view success_msg = "") -> decltype(auto)
    {
        if (inner.has_value())
        {
            if (!success_msg.empty()) log::info("{}", success_msg);
            if constexpr (!std::is_void_v<T>) return inner.value();
        }
        else
        {
            log::fatal("{}", inner.error());
            std::unreachable();
        }
    }

    template <typename SuccessFn, typename ErrorFn>
    auto resolve(SuccessFn&& on_success, ErrorFn&& on_error) -> decltype(auto)
    {
        if (!inner.has_value())
        {
            return std::invoke(std::forward<ErrorFn>(on_error), inner.error());
        }

        if constexpr (std::is_void_v<T>)
            return std::invoke(std::forward<SuccessFn>(on_success));
        else
            return std::invoke(std::forward<SuccessFn>(on_success), inner.value());
    }

    template <typename U = T> requires (!std::is_void_v<U>)
    auto unwrap_or(U fallback) const -> T
    {
        return inner.value_or(std::forward<U>(fallback));
    }

    template <typename Fn>
    auto map(Fn&& func) -> decltype(auto)
    {
        auto next_expected = inner.transform(std::forward<Fn>(func));
        using Val = typename decltype(next_expected)::value_type;
        using Err = typename decltype(next_expected)::error_type;
        return Result<Val, Err>(std::move(next_expected));
    }

    template <typename Fn>
    auto and_then(Fn&& func) -> decltype(auto)
    {
        auto next_expected = inner.and_then(std::forward<Fn>(func));
        using Val = typename decltype(next_expected)::value_type;
        using Err = typename decltype(next_expected)::error_type;
        return Result<Val, Err>(std::move(next_expected));
    }

    template <typename Fn>
    auto or_else(Fn&& func) -> decltype(auto)
    {
        auto next_expected = inner.transform_error(std::forward<Fn>(func));
        using Val = typename decltype(next_expected)::value_type;
        using Err = typename decltype(next_expected)::error_type;
        return Result<Val, Err>(std::move(next_expected));
    }
};

EXPORT_END