#pragma once

#include <native/core/include.h>
#include <native/core/root.h>
#include <native/core/image_barrier.h>
#include <native/utils/obn_vector.h>

struct EnvironmentMap
{
    Vector<int, 2> dimensions;
    int channels;
    std::vector<float> pixels;

    avk::image image;
    avk::image_view view;
    avk::sampler sampler;

    EnvironmentMap() = default;

    static auto load(const std::string& path) -> EnvironmentMap;
    void upload(Root& root);
};