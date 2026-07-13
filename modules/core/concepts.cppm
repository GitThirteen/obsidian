module;
#include <native/macros.h>

// ========================== //
//   CORE - CONCEPTS MODULE   //
// ========================== //

export module Obsidian.Core:Concepts;
import std;

EXPORT(obsidian)

template<typename T>
concept IsNumeric = std::integral<T> || std::floating_point<T>;

template<typename T>
concept IsLikeString = std::is_convertible_v<T, std::string_view>;

template<typename T>
concept IsBinaryContainer = std::ranges::contiguous_range<T> && std::ranges::sized_range<T> && std::is_trivially_copyable_v<std::ranges::range_value_t<T>>;

template<typename T>
concept IsStdVector = std::same_as<T, std::vector<typename T::value_type, typename T::allocator_type>>;

template <typename T>
concept IsMagmaVector = requires {
    typename T::value_type;
    { T::size } -> std::convertible_to<size_t>;
};

EXPORT_END