#version 460
#extension GL_KHR_vulkan_glsl: enable

struct Particle {
    vec4 pos;
    vec4 vel;
};
layout(std430, set = 1, binding = 0) readonly buffer ParticleBuffer {
    Particle particles[];
};

layout(set = 0, binding = 0) uniform CameraData {
    mat4 view_proj;
    vec4 cam_pos;
} camera;

layout(location = 0) out float outAlpha;

void main() {
    uint particleIdx = gl_VertexIndex / 6;
    uint cornerIdx = gl_VertexIndex % 6;

    Particle p = particles[particleIdx];
    
    vec3 start = p.pos.xyz;
    float streak_len = 0.5; // TODO: Scale streak by velocity?
    vec3 end = start - (p.vel.xyz * 0.05); // Tail trails behind velocity
    
    vec3 dir = start - end;
    vec3 viewDir = normalize(start - camera.cam_pos.xyz);
    vec3 right = normalize(cross(dir, viewDir));
    
    // quad offsets
    vec2 offsets[6] = vec2[](
        vec2(-0.5, 0.0), vec2(0.5, 0.0), vec2(-0.5, 1.0),
        vec2(-0.5, 1.0), vec2(0.5, 0.0), vec2(0.5, 1.0)
    );
    vec2 offset = offsets[cornerIdx];
    
    // final position
    float width = 0.02;
    vec3 position = start - (dir * offset.y) + (right * offset.x * width);

    gl_Position = camera.view_proj * vec4(position, 1.0);
    
    outAlpha = 1.0 - offset.y;
}