#version 460
#extension GL_KHR_vulkan_glsl: enable

layout(location = 0) in float inAlpha;
layout(location = 0) out vec4 outColor;

// Unused
layout(set = 0, binding = 0) uniform CameraData {
    mat4 view_proj;
    vec4 cam_pos;
} camera;

layout(set = 0, binding = 1) uniform LightData { vec4 _; } lights; 
layout(set = 0, binding = 2) uniform sampler2D skybox;
layout(set = 0, binding = 3) uniform sampler2D u_shadowMask;

void main() {
    vec3 blue_white = vec3(0.8, 0.9, 1.0);
    outColor = vec4(blue_white, 0.5 * inAlpha);
}