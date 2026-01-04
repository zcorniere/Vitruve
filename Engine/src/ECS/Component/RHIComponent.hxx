#pragma once

#include "RHI/Resources/RHITexture.hxx"

namespace ecs
{

class FRenderTargetComponent
{
    RTTI_DECLARE_TYPEINFO_MINIMAL(FRenderTargetComponent);

public:
    TArray<FRHIRenderTarget> GetColorTargets() const
    {
        if (Viewport)
        {
            return {
                {
                    .Texture = Viewport->GetBackbuffer(),
                    .ClearColor = {0.0f, 0.0f, 0.0f, 1.0f},
                    .LoadAction = ERenderTargetLoadAction::Clear,
                    .StoreAction = ERenderTargetStoreAction::Store,
                },
            };
        }
        else
        {
            return ColorTargets;
        }
    }
    std::optional<FRHIRenderTarget> GetDepthTarget() const
    {
        if (Viewport)
        {
            return FRHIRenderTarget{
                .Texture = Viewport->GetDepthBuffer(),
                .ClearColor = {1.0f, 0.0f, 0.0f, 1.0f},
                .LoadAction = ERenderTargetLoadAction::Clear,
                .StoreAction = ERenderTargetStoreAction::Store,
            };
        }
        else
        {
            return DepthTarget;
        }
    }
    UVector2 GetRenderTargetSize() const
    {
        if (Viewport)
        {
            return Viewport->GetSize();
        }
        else
        {
            return Size;
        }
    }

    bool operator==(const FRenderTargetComponent& Other) const = default;

public:
    /// The associated RHIViewport of this render target. Can be null if the rendering is not linked to an actuall
    /// window If this field is not null ColorTargets, DepthTarget and Size field will be ignored
    Ref<RRHIViewport> Viewport = nullptr;

    /// Texture Holder for colored render textures
    TArray<FRHIRenderTarget> ColorTargets = {};
    /// Optional Texture holder if the render target requires a depth/stencil
    std::optional<FRHIRenderTarget> DepthTarget = std::nullopt;
    /// Size of the render targets
    UVector2 Size = {0, 0};

    /// The index the RHI scene identified this render target.
    /// @note It mustn't be changed manually
    int AssignedSceneIndex = -1;
};

}    // namespace ecs
