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

[[vk::binding(0, 0)]]
Sampler2D    sTexture    : register(t0, space0);

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
    float4 Color    : LOCATION0;
    float2 UV       : LOCATION1;
};

[shader("vertex")]
VSOutput vertexMain(VSInput input)
{
    VSOutput output;

    float r = float((input.aColor >> 16) & 0xFF) / 255.0;
    float g = float((input.aColor >> 8)  & 0xFF) / 255.0;
    float b = float((input.aColor)       & 0xFF) / 255.0;
    float a = float((input.aColor >> 24) & 0xFF) / 255.0;

    output.Color = float4(r, g, b, a);
    output.UV = input.aUV;

    float2 transformed = input.aPos * uScale + uTranslate;

    output.position = float4(transformed, 0.0, 1.0);

    return output;
}

[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_Target0
{
    float4 texColor = sTexture.SampleLevel(input.UV, 0);
    return input.Color * texColor;
}
