#include "ShaderConstants.h"
#include "LightFunctions.hlsli"

#define PI 3.1415926538

struct PS_INPUT
{
    float4 sv_position : SV_POSITION;
    float4 position : POSITION;
    float4 normal : NORMAL;
};

cbuffer CONSTANT_PS_PBR : register(b0)
{
    float3 albedo;
    float alpha;
    float roughness;
    float ior;
    float ambient_occlusion;
    float metalness;
    float4 lights[MAX_NUMBER_LIGHTS];
    float4 camera;
}

// Trowbridge-Reitz GGX
float distribution_ggx(float3 normal, float3 half_dir, float roughness)
{
    float a = roughness * roughness;
    float a_sq = a * a;
    float ndoth = max(dot(normal, half_dir), 0.0);
    float ndoth_sq = ndoth * ndoth;
    float nom = a_sq;
    float denom = ndoth_sq * (a_sq - 1.0) + 1.0;
    denom = PI * denom * denom;
    return nom / max(denom, 0.001);
}

// Schlick Beckmann GGX
float geometry_schlick_ggx(float3 normal, float3 v, float roughness)
{
    float r = roughness + 1.;
    float k = (r * r) / 8.0;
    float ndotv = max(dot(normal, v), 0.0);
    return ndotv / (ndotv * (1.0 - k) + k);
}

// Smith GGX
float geometry_smith_ggx(float3 normal, float3 view, float3 light, float roughness)
{
    float ggx1 = geometry_schlick_ggx(normal, view, roughness);
    float ggx2 = geometry_schlick_ggx(normal, light, roughness);
    return ggx1 * ggx2;
}

float3 fresnel_schlick(float cosine_theta, float3 f0)
{
    return f0 + (1.0 - f0) * pow(1.0 - cosine_theta, 5.0);
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 normal = normalize(input.normal.xyz);
    float3 view_dir = normalize(camera.xyz - input.position.xyz);

    // fresnel from ior
    float f0 = (ior - 1.0) / (ior + 1.0);
    f0 = f0 * f0;
    float3 fresnel_0 = {f0, f0, f0};
    fresnel_0 = lerp(fresnel_0, albedo, metalness);

    float3 radiance_t = {0.0, 0.0, 0.0};

    for (int i = 0; i < MAX_NUMBER_LIGHTS; ++i)
    {
        if (lights[i].w == 0) break; // all remainding are 0

        // per light radiance
        float3 light_pos = lights[i].xyz;
        float3 light_color = {lights[i].w, lights[i].w, lights[i].w};

        float3 light_dir = normalize(light_pos - input.position.xyz);
        float3 half_dir = normalize(view_dir + light_dir);
        float attenuation = calculate_attenuation(input.position.xyz, light_pos);
        float3 radiance = light_color * attenuation;

        // Cook-Torrance BRDF
        float normal_dist = distribution_ggx(normal, half_dir, roughness);
        float geometry = geometry_smith_ggx(normal, view_dir, light_dir, roughness);
        float3 fresnel = fresnel_schlick(clamp(dot(half_dir, view_dir), 0.0, 1.0), fresnel_0);

        float3 nominator = normal_dist * geometry * fresnel;
        float denominator =
            4.0 * max(dot(normal, view_dir), 0.0) * max(dot(normal, light_dir), 0.0);
        float3 specular = nominator / max(denominator, 0.001);

        float3 reflect_ = fresnel;
        float3 refract_ = (float3(1.0, 1.0, 1.0) - reflect_) * (1.0 - metalness);
        float ndotl = max(dot(normal, light_dir), 0.0);
        radiance_t += (refract_ * albedo / PI + specular) * radiance * ndotl;
    }

    float3 ambient = float3(.03, .03, .03) * albedo * ambient_occlusion;
    float3 color = ambient + radiance_t;

    // gamma correction
    color = color / (color + float3(1.0, 1.0, 1.0));
    float exp = 1.0 / 2.2;
    color = pow(color, float3(exp, exp, exp));

    return float4(color, alpha);
}