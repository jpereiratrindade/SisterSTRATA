#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

// UBO Declaration (Same as Vert)
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 lightDir;
    vec3 lightColor;
    float ambientStrength;
} ubo;

void main() {
    // 1. Setup Light (Dynamic from UBO)
    vec3 lightDirection = normalize(ubo.lightDir); 
    vec3 lightCol = ubo.lightColor;
    vec3 objectColor = fragColor;

    // 2. Ambient Component
    vec3 ambient = ubo.ambientStrength * lightCol;

    // 3. Diffuse Component
    vec3 norm = normalize(fragNormal);
    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = diff * lightCol;

    // 4. Combine
    vec3 result = (ambient + diffuse) * objectColor;
    outColor = vec4(result, 1.0);
}
