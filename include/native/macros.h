#pragma once

#define EXPORT_OBSIDIAN export namespace obsidian {
#define EXPORT_END }

#define PADDING_JOIN(x, y) x ## y
#define PADDING_NAME(x, y) PADDING_JOIN(x, y)
#define PADDING(bytes) char PADDING_NAME(_pad_, __LINE__)[bytes]