#version 450 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;

layout(push_constant) uniform uPushConstant
{
    vec2 uScale;
    vec2 uTranslate;
}
pc;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outUV;

void main()
{
    outColor = aColor;
    outUV = aUV;
    gl_Position = vec4(aPos * pc.uScale + pc.uTranslate, 0, 1);
}
