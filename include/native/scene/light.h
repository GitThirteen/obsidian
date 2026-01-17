#pragma once

#include <native/core/include.h>

struct alignas(16) AmbientLight
{ 
    glm::vec3 color = glm::vec3(1.0f);
    float intensity = 0.0f;
};

struct alignas(16) DirectionalLight
{
    glm::vec3 color = glm::vec3(1.0f);
    float intensity = 0.0f;
    glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
    PADDING(4);
};

struct alignas(16) SceneLight
{
    // Pointlight Data
    glm::vec3 color = glm::vec3(1.0f);
    float intensity = 0.0f;
    glm::vec3 position = glm::vec3(0.0f);
    std::uint32_t is_spotlight = 0;
    glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
    float radius = 10.0f;
    // Spotlight Additions
    float inner_cutoff = 12.5f;
    float outer_cutoff = 17.5f;
    float falloff = 1.0f;
    PADDING(4);
};