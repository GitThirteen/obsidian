#version 460
layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec2 inUV;
layout(location = 0) out vec4 outColor;

void main() 
{
    // Map Normal [-1, 1] -> Color [0, 1]
    vec3 color = inNormal * 0.5 + 0.5;
    outColor = vec4(color, 1.0);
}