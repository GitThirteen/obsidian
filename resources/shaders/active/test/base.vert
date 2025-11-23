#version 460
#extension GL_EXT_scalar_block_layout : require

struct Vertex {
    vec3 pos;
    vec3 normal;
    vec2 uv;
};

layout(std430, set = 0, binding = 0) readonly buffer VertexBuffer {
    Vertex vertices[];
};

layout(push_constant) uniform PushConstants {
    mat4 viewProj;
    mat4 model;
} pc;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec2 outUV;

void main() 
{
    Vertex v = vertices[gl_VertexIndex];
    gl_Position = pc.viewProj * pc.model * vec4(v.pos, 1.0);

    outNormal = v.normal;
    outUV = v.uv;
}