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

layout(location = 0) out vec3 fragView;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out mat4 fragTBN;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model *  vec4(inPosition, 1.0);
    fragTexCoord = vec2(inU, inV);
    fragNormal = vec3(normalize(ubo.proj * ubo.view * ubo.model * vec4(inNormal, 0.0)));
    vec3 biTangent = cross(inNormal, inTangent);
    mat4 TBN = mat4(vec4(inTangent, 0.0), vec4(biTangent, 0.0), vec4(inNormal, 0.0), vec4(0.0, 0.0, 0.0,0.0));
    fragTBN = ubo.proj *  ubo.model * TBN;
    fragView = vec3(ubo.view[2]);
}