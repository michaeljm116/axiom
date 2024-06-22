#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D albedoSampler;
const vec4 sun_color = vec4(1.0);
const vec3 sun_direction = normalize(vec3(1.0, 1.0, 1.0));

void main() {
    outColor = sun_color * max(0.0, dot(fragNormal, -sun_direction)) * vec4(fragColor, 1.0) * texture(albedoSampler, fragTexCoord);    
}