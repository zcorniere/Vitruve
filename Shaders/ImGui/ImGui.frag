#version 450 core

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec4 fColor;

layout(set = 0, binding = 0) uniform sampler2D sTexture;

void main()
{
    fColor = inColor * texture(sTexture, inUV.st);
}
