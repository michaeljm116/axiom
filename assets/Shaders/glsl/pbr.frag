#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D albedoSampler;
layout(binding = 2) uniform sampler2D metallicSampler;
layout(binding = 3) uniform sampler2D roughnessSampler;
layout(binding = 4) uniform sampler2D normalSampler;

layout(binding=5) uniform Material{
    vec4 albedo;
    float metalic;
    float roughness;
    bool has_albedo;
    bool has_metallic;
    bool has_roughness;
    bool has_normal;
}material_data;

const vec4 sun_color = vec4(1.0);
const vec3 sun_direction = normalize(vec3(1.0, 1.0, 1.0));

void main() {
    if(material_data.has_normal) 
        outColor = sun_color * max(0.0, dot(texture(normalSampler, fragTexCoord), vec4(-sun_direction.xyz, 0.0))) * vec4(fragColor, 1.0) * texture(albedoSampler, fragTexCoord);        
    else
        outColor = sun_color * max(0.0, dot(fragNormal, -sun_direction)) * vec4(fragColor, 1.0) * texture(albedoSampler, fragTexCoord);    
}