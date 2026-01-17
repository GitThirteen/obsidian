#version 460
#extension GL_KHR_vulkan_glsl: enable

layout(set = 0, binding = 0) uniform CameraData {
    mat4 viewProj;
    vec4 globalCameraPos;
};

layout(location = 0) out vec3 vWorldRay;

const float EPSILON = 0.0001;

void main() 
{
    // Winding order fix
    int idx = int[](0, 2, 1)[gl_VertexIndex];
    vec2 uv = vec2((idx << 1) & 2, idx & 2);

    // Reverse Z shenanigans
    vec4 screenPos = vec4(uv * 2.0 - 1.0, EPSILON, 1.0);

    // Unproject to World Space
    vec4 worldPos = inverse(viewProj) * screenPos;
    worldPos /= worldPos.w;
    
    vec3 rayDir = normalize(worldPos.xyz - globalCameraPos.xyz);
    vWorldRay = rayDir;
    gl_Position = screenPos;
}