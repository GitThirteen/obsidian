#version 460
#extension GL_KHR_vulkan_glsl: enable

layout(location = 0) in vec3 vWorldRay;
layout(location = 0) out vec4 outColor;

struct SceneLight {
    vec4 lighting;
    vec4 position;
    vec4 direction;
    vec4 data;
};

layout(set = 0, binding = 0) uniform CameraData {
    mat4 viewProj;
    vec4 globalCameraPos;
};

layout(set = 0, binding = 1) uniform LightBlock {
    vec4 ambientLighting;
    vec4 sunLighting;
    vec4 sunDirection;
    SceneLight sceneLights[16];
} u_lights;

layout(set = 0, binding = 2) uniform sampler2D u_envMap;

// Unused, but required by layout
layout(set = 0, binding = 3) uniform sampler2D u_shadowMask;

const vec2 invAtan = vec2(0.1591, 0.3183);

vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    uv.y = 1.0 - uv.y; // Vulkan's Y points downwards
    return uv;
}

void main()
{
    vec2 uv = SampleSphericalMap(normalize(vWorldRay));
    vec3 color = texture(u_envMap, uv).rgb;

    // Tone mapping (move into dedicated postprocess shader -> see base shader)
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    outColor = vec4(color, 1.0);
}