#version 460
#extension GL_KHR_vulkan_glsl: enable

layout(location = 0) in vec2 vUV;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec3 vWorldPos;

layout(location = 0) out vec4 outColor;

struct SceneLight {
    vec4 lighting;      // [r, g, b, intensity]
    vec4 position;      // [x, y, z, is_spotlight]
    vec4 direction;     // [x, y, z, radius]
    vec4 data;          // [inner cutoff, outer cutoff, falloff, unused]
};

layout(set = 0, binding = 0) uniform CameraData {
    mat4 viewProj;
    vec4 globalCameraPos;
};

layout(set = 0, binding = 1) uniform LightBlock {
    vec4 ambientLighting; // [r, g, b, intensity]
    vec4 sunLighting;
    vec4 sunDirection;
    SceneLight sceneLights[16];
} u_lights;

layout(set = 1, binding = 0) uniform sampler2D texAlbedo;
layout(set = 1, binding = 1) uniform sampler2D texRoughness;
layout(set = 1, binding = 2) uniform sampler2D texNormal;
layout(set = 1, binding = 3) uniform sampler2D texAO;

layout(set = 1, binding = 4) uniform MaterialBlock {
    vec4 baseColor;
    vec4 params; // [metallic, roughness, unused, unused]
} u_Material;

layout(push_constant) uniform PushConstants {
    mat4 model;
    vec4 localCameraPos;
} u_pc;

const float PI = 3.14159265359;
const float EPSILON = 0.0001;

// Info: This GGX is completely based on https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf

// D: Normal Distribution Function (Trowbridge-Reitz / Disney GGX)
// Note: We omit the positive characteristic function as it is redundant in normal scenarios.
// @Future me: Consider re-adding it if you ever have H inverted (transmissions, ...).
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (alpha2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return (alpha2 / max(denom, EPSILON));
}

// G: Geometry Function (Schlick-GGX)
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    
    float denom = NdotV * (1.0 - k) + k;
    return NdotV / max(denom, EPSILON);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

// F: Fresnel approximation (Schlick-GGX + SG)
vec3 FresnelSchlick(vec3 F0, vec3 V, vec3 H) {
    float VdotH = max(dot(V, H), 0.0);

    return F0 + (1.0 - F0) * exp2((-5.55473 * VdotH - 6.98316) * VdotH);
}

vec3 Evaluate(vec3 V, vec3 L, vec3 N, vec3 radiance, vec3 albedo, float roughness, float metallic, vec3 F0) {
    vec3 H = normalize(V + L);
    
    float D = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = FresnelSchlick(F0, V, H);

    vec3 num = D * G * F;
    float denom = max(4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0), EPSILON);
    vec3 specular = num / denom;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= (1.0 - metallic);

    float NdotL = max(dot(N, L), 0.0);
    vec3 lambDiffuse = albedo / PI;
    return (kD * lambDiffuse + specular) * radiance * NdotL;
}

void main() {
    vec4 rawAlbedo = texture(texAlbedo, vUV) * u_Material.baseColor;
    vec3 albedo = rawAlbedo.rgb;

    float roughness = texture(texRoughness, vUV).r * u_Material.params.y;
    roughness = clamp(roughness, 0.05, 1.0);

    float metallic = u_Material.params.x; 
    float ao = texture(texAO, vUV).r;

    vec3 N = normalize(vNormal); // TODO: sample normal map
    vec3 V = normalize(globalCameraPos.xyz - vWorldPos);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    // 1. DIRECTIONAL LIGHT
    {
        vec3 L = normalize(u_lights.sunDirection.xyz);
        vec3 radiance = u_lights.sunLighting.rgb * u_lights.sunLighting.w;

        Lo += Evaluate(V, L, N, radiance, albedo, roughness, metallic, F0);
    }

    // 2. SCENE LIGHTS (POINT & SPOT)
    for (int i = 0; i < 16; i++) {
        if (u_lights.sceneLights[i].lighting.w <= 0.0) continue;

        vec3 rawL = u_lights.sceneLights[i].position.xyz - vWorldPos;
        float dist = length(rawL);
        vec3 L = normalize(rawL);

        float radius = u_lights.sceneLights[i].direction.w;
        float attenuation = 1.0 / (dist * dist);
        // if(radius > 0.0) attenuation *= clamp(1.0 - (dist / radius), 0.0, 1.0);

        vec3 radiance = u_lights.sceneLights[i].lighting.rgb * u_lights.sceneLights[i].lighting.w * attenuation;

        if(u_lights.sceneLights[i].position.w != 0) { // spotlight
            vec3 spotDir = normalize(u_lights.sceneLights[i].direction.xyz);
            float theta = dot(L, -spotDir);
            float inner = u_lights.sceneLights[i].data.x;
            float outer = u_lights.sceneLights[i].data.y;
            float delta = inner - outer;
            float intensity = clamp((theta - outer) / delta, 0.0, 1.0);
            radiance *= intensity;
        }

        Lo += Evaluate(V, L, N, radiance, albedo, roughness, metallic, F0);
    }
    
    // 3. SCENE LIGHTS (POINT & SPOT)
    vec3 ambient = u_lights.ambientLighting.rgb * u_lights.ambientLighting.w * albedo * ao;
    vec3 color = ambient + Lo;

    // TONE CORRECTION (Important @Future Me: Move this to a separate post-processing shader in the whole pipeline, also maybe look into ACES color correction?)
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    outColor = vec4(color, rawAlbedo.a);
}