#pragma once

/// The type of RHI resource
enum class ERHIResourceType : uint8
{
    None = 0,

    Texture,
    Buffer,
    Shader,
    Viewport,
    Material,
    GraphicsPipeline,

    MAX_VALUE,
};

/// Represent and abstract above RHI resources
class RRHIResource : public RObject
{
    RTTI_DECLARE_TYPEINFO(RRHIResource, RObject);

public:
    RRHIResource() = delete;
    explicit RRHIResource(ERHIResourceType InResourceType): ResourceType(InResourceType)
    {
    }

    virtual ~RRHIResource() = default;

protected:
    const ERHIResourceType ResourceType;
};

// IWYU pragma: begin_exports
#include "Engine/RHI/Resources/RHIBuffer.hxx"
#include "Engine/RHI/Resources/RHIGraphicsPipeline.hxx"
#include "Engine/RHI/Resources/RHIShader.hxx"
#include "Engine/RHI/Resources/RHITexture.hxx"
#include "Engine/RHI/Resources/RHIViewport.hxx"

#include "Engine/RHI/Resources/RHIMaterial.hxx"
// IWYU pragma: end_exports
