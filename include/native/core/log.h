#pragma once

#pragma warning(push)

#pragma warning(disable : 4996)
#define LOGURU_WITH_STREAMS 1
#include <external/loguru/loguru.hpp>

#pragma warning(pop)

struct Logger
{
    static void initialize(int& argc, char** argv, const std::string& filename)
    {
        loguru::g_preamble_uptime = false;
        loguru::g_preamble_thread = false;

        loguru::init(argc, argv);
        loguru::add_file(filename.c_str(), loguru::Truncate, loguru::Verbosity_INFO);
    }
};