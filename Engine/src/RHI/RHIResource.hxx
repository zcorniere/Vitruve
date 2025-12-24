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
#include "RHI/Resources/RHIBuffer.hxx"
#include "RHI/Resources/RHIGraphicsPipeline.hxx"
#include "RHI/Resources/RHIShader.hxx"
#include "RHI/Resources/RHITexture.hxx"
#include "RHI/Resources/RHIViewport.hxx"

#include "RHI/Resources/RHIMaterial.hxx"
// IWYU pragma: end_exports
