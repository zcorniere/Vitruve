// =====================================================
// Set 0 - Scene information
// =====================================================

struct PointLight
{
    float3 Position;
    float Multiplier;
    float3 Radiance;
    float MinRadius;
    float Radius;
    float Falloff;
    float LightSize;
    uint CastsShadows;
};

cbuffer PointLights : register(b0, space0)
{
    uint PointLightCount;
    PointLight PointLights[1024];
};

struct DirectionalLight
{
    float3 Direction;
    float ShadowAmount;
    float3 Radiance;
    float Multiplier;
};

cbuffer DirectionalLights : register(b1, space0)
{
    uint DirectionalLightCount;
    DirectionalLight DirectionalLights[4];
};

// =====================================================
// Set 1 - Rendering data
// =====================================================

cbuffer Camera : register(b0, space1)
{
    row_major float4x4 Viewproj;
    row_major  float4x4 View;
    row_major float4x4 Proj;
};

// =====================================================
// Vertex Input / Output
// =====================================================

struct VSInput
{
    float3 inPosition      : LOCATION0;
    float3 inNormal        : LOCATION1;
    float3 inTangent       : LOCATION2;
    float3 inBitangent     : LOCATION3;
    float2 inTexCoord      : LOCATION4;

    row_major float4x4 inTransformMatrix : LOCATION5;
};

struct VSOutput
{
    float4 position : SV_Position;

    float3 outPosition : LOCATION0;
    float3 outNormal   : LOCATION1;
};

[shader("vertex")]
VSOutput vertexMain(VSInput input)
{
    VSOutput output;

    output.outPosition = input.inPosition;
    output.outNormal   = input.inNormal;

    float4 worldPosition = mul(input.inTransformMatrix, float4(input.inPosition, 1.0));
    output.position = mul(Viewproj, worldPosition);

    return output;
}

float3 ComputePointLightParticipation(PointLight Light, float3 inPosition, float3 inNormal)
{
    float distance = length(Light.Position - inPosition);

    float attenuation = saturate(
        1.0 - (distance * distance) / (Light.Radius * Light.Radius)
    );

    attenuation *= lerp(attenuation, 1.0, Light.Falloff);

    float3 Lradiance = Light.Radiance * Light.Multiplier * attenuation;

    // Ambient
    float ambientStrength = 0.1;
    float3 ambient = ambientStrength * Lradiance;

    // Diffuse
    float3 norm = normalize(inNormal);
    float3 lightDir = normalize(Light.Position - inPosition);

    float diff = max(dot(norm, lightDir), 0.0);
    float3 diffuse = diff * Lradiance;

    // Specular
    float specularStrength = 0.5;
    float3 reflectDir = reflect(-lightDir, norm);

    float spec = pow(max(dot(norm, reflectDir), 0.0), 32);

    float3 specular = specularStrength * spec * Lradiance;

    return ambient + ((diffuse + specular) * Light.Multiplier);
}

float3 ComputeDirectionalLightParticipation(
    DirectionalLight Light,
    float3 inPosition,
    float3 inNormal)
{
    float ambientStrength = 0.1;
    float3 ambient = ambientStrength * Light.Radiance;

    float3 norm = normalize(inNormal);
    float3 lightDir = normalize(-Light.Direction);

    float diff = max(dot(norm, lightDir), 0.0);
    float3 diffuse = diff * Light.Radiance;

    float specularStrength = 0.5;
    float3 reflectDir = reflect(-lightDir, norm);

    float spec = pow(max(dot(norm, reflectDir), 0.0), 32);

    float3 specular = specularStrength * spec * Light.Radiance;

    return ambient + ((diffuse + specular) * Light.Multiplier);
}

struct PSInput
{
    float3 inPosition : LOCATION0;
    float3 inNormal   : LOCATION1;
};

[shader("fragment")]
float4 fragmentMain(PSInput input) : SV_Target0
{
    float3 pointLightAccum = float3(0,0,0);

    [loop]
    for (uint i = 0; i < PointLightCount; i++)
    {
        pointLightAccum += ComputePointLightParticipation(
            PointLights[i],
            input.inPosition,
            input.inNormal
        );
    }

    float3 directionalAccum = float3(0,0,0);

    [loop]
    for (uint i = 0; i < DirectionalLightCount; i++)
    {
        directionalAccum += ComputeDirectionalLightParticipation(
            DirectionalLights[i],
            input.inPosition,
            input.inNormal
        );
    }

    return float4(pointLightAccum + directionalAccum, 1.0);
}
