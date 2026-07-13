#pragma once

#define OBSIDIAN_EXPAND(x) x

#define OBSIDIAN_NS_1(a) a
#define OBSIDIAN_NS_2(a, b) a::b
#define OBSIDIAN_NS_3(a, b, c) a::b::c
#define OBSIDIAN_NS_4(a, b, c, d) a::b::c::d

#define OBSIDIAN_GET_NS_MACRO(_1, _2, _3, _4, NAME, ...) NAME

#define OBSIDIAN_JOIN_NS(...) OBSIDIAN_EXPAND(OBSIDIAN_GET_NS_MACRO(__VA_ARGS__, OBSIDIAN_NS_4, OBSIDIAN_NS_3, OBSIDIAN_NS_2, OBSIDIAN_NS_1)(__VA_ARGS__))

#define NAMESPACE(...) namespace OBSIDIAN_JOIN_NS(__VA_ARGS__) {
#define EXPORT(...) export namespace OBSIDIAN_JOIN_NS(__VA_ARGS__) {
#define NAMESPACE_END }
#define EXPORT_END }

#define PADDING_JOIN(x, y) x ## y
#define PADDING_NAME(x, y) PADDING_JOIN(x, y)
#define PADDING(bytes) char PADDING_NAME(_pad_, __LINE__)[bytes]