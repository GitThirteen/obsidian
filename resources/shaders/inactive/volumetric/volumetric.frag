#version 460
layout(location = 0) in vec3 vUVW;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform CameraData {
    mat4 viewProj;
    vec4 globalCameraPos;
};

layout(binding = 1) uniform sampler3D volumeTex;

layout(push_constant) uniform PushConstants {
    mat4 model;
    vec4 localCameraPos;
} u_pc;

const int STEPS = 128;
const float DENSITY_MULTIPLIER = 5.0;

vec2 intersect_aabb(vec3 rayOrigin, vec3 rayDir, vec3 boxMin, vec3 boxMax) {
    vec3 tMin = (boxMin - rayOrigin) / rayDir;
    vec3 tMax = (boxMax - rayOrigin) / rayDir;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
    return vec2(tNear, tFar);
}

void main() {
    vec3 rayDir = normalize(vUVW - u_pc.localCameraPos.xyz);
    vec2 t = intersect_aabb(u_pc.localCameraPos.xyz, rayDir, vec3(0.0), vec3(1.0));
  
    float tStart = max(t.x, 0.0);
    float tEnd = max(t.y, 0.0);
    
    float totalDist = tEnd - tStart;
    if (totalDist <= 0.0) discard; // Missed the box entirely
    
    float stepSize = totalDist / float(STEPS);
    vec3 currentPos = u_pc.localCameraPos.xyz + rayDir * tStart;
    vec3 deltaStep = rayDir * stepSize;
    
    vec4 accum = vec4(0.0); // rgb = color, a = opacity

    // Simple Ray Marching
    for (int i = 0; i < STEPS; i++) {
        float density = texture(volumeTex, currentPos).r;

        if (density > 0.01) {
            vec3 color = vec3(density); 
            float alpha = density * DENSITY_MULTIPLIER * stepSize;
            
            accum.rgb += (1.0 - accum.a) * color * alpha;
            accum.a   += (1.0 - accum.a) * alpha;
        }
        
        currentPos += deltaStep;
    }

    if (accum.a < 0.001) {
        discard; 
    }
    
    outColor = accum;
}