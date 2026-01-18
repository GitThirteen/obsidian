#version 460
#extension GL_KHR_vulkan_glsl: enable

layout(location = 0) in vec2 vUV;
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

// Unused, but required by layout
layout(set = 0, binding = 2) uniform sampler2D u_envMap;
layout(set = 0, binding = 3) uniform sampler2D u_shadowMask;

float noise(vec2 t) {
    return fract(sin(dot(t, vec2(12.9898, 78.233))) * 43758.5453);
}

vec3 lensflare(vec2 uv, vec2 pos)
{
    vec2 main = uv - pos;
    vec2 uvd = uv * (length(uv));
    
    float ang = atan(main.x, main.y);
    float dist = length(main); 
    dist = pow(dist, 0.1);
    
    float n = noise(vec2(ang * 16.0, dist * 32.0));
    
    float f0 = 1.0 / (length(uv - pos) * 16.0 + 1.0);
    
    f0 = f0 + f0 * (sin(noise(vec2(sin(ang * 2.0 + pos.x) * 4.0 - cos(ang * 3.0 + pos.y))) * 16.0) * 0.1 + dist * 0.1 + 0.8);
    
    float f1 = max(0.01 - pow(length(uv + 1.2 * pos), 1.9), 0.0) * 7.0;

    float f2 = max(1.0 / (1.0 + 32.0 * pow(length(uvd + 0.8 * pos), 2.0)), 0.0) * 0.25;
    float f22 = max(1.0 / (1.0 + 32.0 * pow(length(uvd + 0.85 * pos), 2.0)), 0.0) * 0.23;
    float f23 = max(1.0 / (1.0 + 32.0 * pow(length(uvd + 0.9 * pos), 2.0)), 0.0) * 0.21;
    
    vec2 uvx = mix(uv, uvd, -0.5);
    
    float f4 = max(0.01 - pow(length(uvx + 0.4 * pos), 2.4), 0.0) * 6.0;
    float f42 = max(0.01 - pow(length(uvx + 0.45 * pos), 2.4), 0.0) * 5.0;
    float f43 = max(0.01 - pow(length(uvx + 0.5 * pos), 2.4), 0.0) * 3.0;
    
    uvx = mix(uv, uvd, -0.4);
    
    float f5 = max(0.01 - pow(length(uvx + 0.2 * pos), 5.5), 0.0) * 2.0;
    float f52 = max(0.01 - pow(length(uvx + 0.4 * pos), 5.5), 0.0) * 2.0;
    float f53 = max(0.01 - pow(length(uvx + 0.6 * pos), 5.5), 0.0) * 2.0;
    
    uvx = mix(uv, uvd, -0.5);
    
    float f6 = max(0.01 - pow(length(uvx - 0.3 * pos), 1.6), 0.0) * 6.0;
    float f62 = max(0.01 - pow(length(uvx - 0.325 * pos), 1.6), 0.0) * 3.0;
    float f63 = max(0.01 - pow(length(uvx - 0.35 * pos), 1.6), 0.0) * 5.0;
    
    vec3 c = vec3(0.0);
    
    c.r += f2 + f4 + f5 + f6; 
    c.g += f22 + f42 + f52 + f62; 
    c.b += f23 + f43 + f53 + f63;
    c = c * 1.3 - vec3(length(uvd) * 0.05);
    c += vec3(f0);
    
    return c;
}

vec3 cc(vec3 color, float factor, float factor2)
{
    float w = color.x + color.y + color.z;
    return mix(color, vec3(w) * factor, w * factor2);
}

vec3 WorldToScreen(vec3 worldPos, float aspect)
{
    vec4 clipPos = viewProj * vec4(worldPos, 1.0);
    if (clipPos.w <= 0.0) return vec3(0.0);

    vec3 ndc = clipPos.xyz / clipPos.w;

    vec2 screen = ndc.xy * 0.5;
    screen.x *= aspect;
    
    return vec3(screen, 1.0);
}

void main()
{
    float aspect = globalCameraPos.w;
    
    vec2 uv = vUV - 0.5;
    uv.x *= aspect;
    
    vec3 totalColor = vec3(0.0);

    vec3 sunWorldPos = globalCameraPos.xyz + normalize(u_lights.sunDirection.xyz) * 1000.0;
    vec3 sunScreen = WorldToScreen(sunWorldPos, aspect);
    
    if (sunScreen.z > 0.0)
    {
        // Note @Future Me: Real occlusion requires a depth buffer check here. We simulate this for now.
        vec3 flare = lensflare(uv, sunScreen.xy);
        
        float intensity = u_lights.sunLighting.w * 0.01;
        totalColor += flare * u_lights.sunLighting.rgb * intensity;
    }

    for (int i = 0; i < 16; i++)
    {
        if (u_lights.sceneLights[i].lighting.w <= 0.0) continue;

        vec3 lightScreen = WorldToScreen(u_lights.sceneLights[i].position.xyz, aspect);
        
        if (lightScreen.z > 0.0) 
        {
            vec3 flare = lensflare(uv, lightScreen.xy);
            float intensity = u_lights.sceneLights[i].lighting.w * 0.005;
            totalColor += flare * u_lights.sceneLights[i].lighting.rgb * intensity;
        }
    }

    totalColor -= noise(gl_FragCoord.xy) * 0.015;
    totalColor = cc(totalColor, 0.5, 0.1);
    
    outColor = vec4(totalColor, 1.0);
}