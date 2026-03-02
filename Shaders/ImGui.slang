// =====================================================
// Push constants (Vulkan)
// =====================================================

[[vk::push_constant]]
cbuffer PushConstants
{
    float2 uScale;
    float2 uTranslate;
};

// =====================================================
// Texture (set = 0, binding = 0)
// =====================================================

Texture2D    sTexture    : register(t0, space0);
SamplerState sSampler    : register(s0, space0);

// =====================================================
// Vertex structures
// =====================================================

struct VSInput
{
    float2 aPos   : LOCATION0;
    float2 aUV    : LOCATION1;
    uint   aColor : LOCATION2;
};

struct VSOutput
{
    float4 position : SV_Position;
    float4 outColor : LOCATION0;
    float2 outUV    : LOCATION1;
};

[shader("vertex")]
VSOutput vertexMain(VSInput input)
{
    VSOutput output;

    // unpack ABGR (same logic as GLSL)
    float r = float((input.aColor >> 16) & 0xFF) / 255.0;
    float g = float((input.aColor >> 8)  & 0xFF) / 255.0;
    float b = float((input.aColor)       & 0xFF) / 255.0;
    float a = float((input.aColor >> 24) & 0xFF) / 255.0;

    output.outColor = float4(r, g, b, a);
    output.outUV = input.aUV;

    float2 transformed = input.aPos * uScale + uTranslate;

    output.position = float4(transformed, 0.0, 1.0);

    return output;
}

// =====================================================
// Fragment
// =====================================================

struct PSInput
{
    float4 inColor : LOCATION0;
    float2 inUV    : LOCATION1;
};

[shader("fragment")]
float4 fragmentMain(PSInput input) : SV_Target0
{
    float4 texColor = sTexture.Sample(sSampler, input.inUV);
    return input.inColor * texColor;
}
