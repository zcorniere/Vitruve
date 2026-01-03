#pragma once

/// Set 0 - Scene information
struct PointLight
{
    vec3 Position;
    float Multiplier;
    vec3 Radiance;
    float MinRadius;
    float Radius;
    float Falloff;
    float LightSize;
    bool CastsShadows;
};

layout(std140, set = 0, binding = 0) uniform PointLights
{
    uint LightCount;
    PointLight Lights[1024];
}
u_PointLights;

struct DirectionalLight
{
    vec3 Direction;
    float ShadowAmount;
    vec3 Radiance;
    float Multiplier;
};
layout(std140, set = 0, binding = 1) uniform DirectionalLights
{
    uint LightCount;
    DirectionalLight Lights[1024];
}
u_DirectionalLights;

/// Set 1 - Rendering data
layout(std140, set = 1, binding = 0) uniform Camera
{
    mat4 Viewproj;
    mat4 View;
    mat4 Proj;
}
u_Camera;
