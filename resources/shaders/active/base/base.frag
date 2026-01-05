#version 460
layout(location = 0) in vec2 vUV;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec3 vWorldPos;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform CameraData {
    mat4 viewProj;
    vec4 globalCameraPos;
};

layout(binding = 1) uniform sampler2D texAlbedo;
layout(binding = 2) uniform sampler2D texRoughness;
layout(binding = 3) uniform sampler2D texNormal;
layout(binding = 4) uniform sampler2D texAO;

layout(binding = 5) uniform MaterialBlock {
    vec4 baseColor;
    vec4 params; // x = metallic, y = roughness
} u_Material;

layout(push_constant) uniform PushConstants {
    mat4 model;
    vec4 localCameraPos;
} u_pc;

// Hardcoded for now
const vec3 LIGHT_DIR = normalize(vec3(0.5, 1.0, 0.3));
const vec3 LIGHT_COLOR = vec3(1.0);
const vec3 AMBIENT_COLOR = vec3(0.03);

// TODO: Cook-Torrance GGX Shader
void main()
{
    vec4 albedoRaw = texture(texAlbedo, vUV) * u_Material.baseColor;
    vec3 albedo = albedoRaw.rgb;
    float roughness = texture(texRoughness, vUV).r * u_Material.params.y;
    float metallic = u_Material.params.x; 
    float ao = texture(texAO, vUV).r;
    vec3 ignoredNormalSample = texture(texNormal, vUV).rgb;
    vec3 N = normalize(vNormal);

    // --- LIGHTING CALCULATION (Simplified PBR-style) ---

    // Diffuse Factor (Lambert)
    float diff = max(dot(N, LIGHT_DIR), 0.0);

    // Specular Factor (Blinn-Phong approximation for Roughness)
    vec3 viewDir = normalize(-vWorldPos); // Approximate view from origin
    vec3 halfwayDir = normalize(LIGHT_DIR + viewDir);
    float specPower = max((1.0 - roughness) * 64.0, 1.0); // Roughness to shininess
    float spec = pow(max(dot(N, halfwayDir), 0.0), specPower);

    // Metallic Logic:
    // Metals have no diffuse reflection (they absorb it), only specular.
    // Their specular color is their Albedo.
    // Non-metals (dielectrics) have white specular and colored diffuse.
    
    vec3 kS = mix(vec3(0.04), albedo, metallic); // Specular Color
    vec3 kD = vec3(1.0) - kS;                    // Diffuse weight
    kD *= (1.0 - metallic);                      // Metals have 0 diffuse

    // Final Composition
    vec3 ambient = AMBIENT_COLOR * albedo * ao;
    vec3 diffuse = kD * albedo * diff * LIGHT_COLOR;
    vec3 specular = kS * spec * LIGHT_COLOR;

    vec3 finalColor = ambient + diffuse + specular;

    // Gamma correction
    finalColor = pow(finalColor, vec3(1.0 / 2.2));

    outColor = vec4(finalColor, albedoRaw.a);
}