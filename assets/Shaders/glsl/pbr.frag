#version 450

#define PI 3.1415926535897932384626433832795
#define INVPI 0.318309886184

layout(location = 0) in vec3 fragView;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in mat4 fragTBN;

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
const vec3 sun_direction = -normalize(vec3(0.99, 0.99, 1.0));


///////////////////////////////////pbr/////////////////////////////////////
///////////////////////////////////pbr/////////////////////////////////////
///////////////////////////////////pbr/////////////////////////////////////
// Normal Distribution function --------------------------------------
float D_GGX(float dotNH, float roughness)
{
	float alpha = roughness * roughness;
	float alpha2 = alpha * alpha;
	float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
	return (alpha2)/(PI * denom*denom); 
}

// Geometric Shadowing function --------------------------------------
float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;
	float GL = dotNL / (dotNL * (1.0 - k) + k);
	float GV = dotNV / (dotNV * (1.0 - k) + k);
	return GL * GV;
}

// Fresnel function ----------------------------------------------------
vec3 F_Schlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
vec3 F_SchlickR(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}
vec3 specularContribution(vec3 L, vec3 V, vec3 N, vec3 F0, vec3 albedo, float metallic, float roughness)
{
	// Precalculate vectors and dot products	
	vec3 H = normalize (V + L);
	float dotNH = clamp(dot(N, H), 0.0, 1.0);
	float dotNV = clamp(dot(N, V), 0.0, 1.0);
	float dotNL = clamp(dot(N, L), 0.0, 1.0);

	// Light color fixed
	vec3 lightColor = vec3(1.0);

	vec3 color = vec3(0.0);

	if (dotNL > 0.0) {
		// D = Normal distribution (Distribution of the microfacets)
		float D = D_GGX(dotNH, roughness); 
		// G = Geometric shadowing term (Microfacets shadowing)
		float G = G_SchlicksmithGGX(dotNL, dotNV, roughness);
		// F = Fresnel factor (Reflectance depending on angle of incidence)
		//vec3 F = F_Schlick(dotNV, F0);		
		vec3 F = F_SchlickR(dotNV, F0, roughness);
		vec3 spec = D * F * G / (4.0 * dotNL * dotNV + 0.001);		
		vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);			
		color += (kD * albedo * INVPI + spec) * dotNL;
	}

	return color;
}

///////////////////////////////////pbr/////////////////////////////////////
///////////////////////////////////pbr/////////////////////////////////////
///////////////////////////////////pbr/////////////////////////////////////

void main() {
    vec3 albedo = material_data.albedo.xyz * vec3(texture(albedoSampler, fragTexCoord));
    vec3 metallic = vec3(texture(metallicSampler, fragTexCoord));
    vec3 roughness = vec3(texture(roughnessSampler, fragTexCoord));
    vec3 normal = vec3(texture(normalSampler, fragTexCoord));
    normal = normalize(normal);
    vec3 new_sun_direction = sun_direction * mat3(fragTBN);
    vec3 diffuse = max(0.0, dot(normal, new_sun_direction)) * albedo;
    vec3 reflective = 2*(dot(normal, new_sun_direction)) * normal - new_sun_direction;
    vec3 f0 = vec3(0.04);
    f0 = mix(f0, diffuse, reflective);
    float metal = material_data.metalic * metallic.y;
    float rough = material_data.roughness * roughness.z;
    vec3 specular =  specularContribution(new_sun_direction, fragView, normal, f0, albedo, metal, rough);
    outColor = sun_color * vec4(diffuse + specular, 1.0);

    //vec4 normalized_sampler =  fma(texture(normalSampler, fragTexCoord) , one_over_128, v_neg_one);
    //vec4 transformed_sun_direction = sun_direction * fragTBN;
    //outColor = sun_color * max(0.0, dot(normalized_sampler, transformed_sun_direction)) * texture(albedoSampler, fragTexCoord);
    
    //if(!material_data.has_normal) 
    //    outColor = sun_color * max(0.0, dot(normalized_sampler, -sun_direction)) * vec4(fragColor, 1.0) * texture(albedoSampler, fragTexCoord);        
    //else
    //   outColor = sun_color * max(0.0, dot(fragNormal, -sun_direction)) * vec4(fragColor, 1.0) * texture(albedoSampler, fragTexCoord);    
}




