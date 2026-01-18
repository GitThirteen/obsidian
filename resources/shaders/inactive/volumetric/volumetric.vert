#version 460
layout(location = 0) in vec3 inPos; // Object Space (-0.5 to 0.5)
layout(location = 0) out vec3 vUVW; // Texture Space (0.0 to 1.0)

layout(binding = 0) uniform CameraData {
    mat4 viewProj;
    vec4 globalCameraPos;
};

layout(push_constant) uniform PushConstants {
    mat4 model;
    vec4 localCameraPos;
} u_pc;

void main() {
    // Map -0.5..0.5 to 0.0..1.0 for texture sampling
    vUVW = inPos + 0.5;
    
    gl_Position = viewProj * u_pc.model * vec4(inPos, 1.0);
}