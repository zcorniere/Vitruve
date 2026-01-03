#version 450
#extension GL_ARB_shading_language_include : require

#include <include/Buffers.glsl>

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

vec3 ComputePointLightParticipation(PointLight Light)
{
    // attenuation
    float distance = length(Light.Position - inPosition);
    float attenuation = clamp(1.0 - (distance * distance) / (Light.Radius * Light.Radius), 0.0, 1.0);
    attenuation *= mix(attenuation, 1.0, Light.Falloff);
    vec3 Lradiance = Light.Radiance * Light.Multiplier * attenuation;

    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * Lradiance;

    // diffuse
    vec3 norm = normalize(inNormal);
    vec3 lightDir = normalize(Light.Position - inPosition);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * Lradiance;

    // specular
    float specularStrength = 0.5;
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(lightDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * Lradiance;

    return (ambient + ((diffuse + specular) * Light.Multiplier));
}

void main()
{
    vec3 PointLight = vec3(0);
    for (int i = 0; i < u_PointLights.LightCount; i++)
    {
        PointLight += ComputePointLightParticipation(u_PointLights.Lights[i]);
    }
    outColor = vec4(PointLight, 1.0);
}
