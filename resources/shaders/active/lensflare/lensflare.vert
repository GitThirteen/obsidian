#version 460
#extension GL_KHR_vulkan_glsl: enable

layout(location = 0) out vec2 vUV;

layout(set = 0, binding = 0) uniform CameraData {
    mat4 viewProj;
    vec4 globalCameraPos;
};

void main() 
{
    // Winding order fix
    int idx = int[](0, 2, 1)[gl_VertexIndex];
    vec2 uv = vec2((idx << 1) & 2, idx & 2);

    vUV = uv;
    gl_Position = vec4(vUV * 2.0 - 1.0, 1.0, 1.0);
}