#version 450

layout(binding = 0) uniform UBO{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in float inU;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in float inV;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in float inPad;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model *  vec4(inPosition, 1.0);
    fragColor = vec3(1.0);
    fragTexCoord = vec2(inU, inV);
}