#version 460
#extension GL_EXT_scalar_block_layout : require

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec2 vUV;
layout(location = 1) out vec3 vNormal;
layout(location = 2) out vec3 vWorldPos;

layout(binding = 0) uniform CameraData {
    mat4 viewProj;
    vec4 globalCameraPos;
};

layout(push_constant) uniform PushConstants {
    mat4 model;
    vec4 localCameraPos;
} u_pc;

void main() 
{
    vec4 worldPos = u_pc.model * vec4(inPos, 1.0);
    vWorldPos = worldPos.xyz;

    vUV = inUV;

    vNormal = normalize(mat3(transpose(inverse(u_pc.model))) * inNormal);

    gl_Position = viewProj * worldPos;
}