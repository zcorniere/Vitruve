#pragma once

#include "Engine/Core/RHI/RHIContext.hxx"

#include "VulkanRHI/VulkanPendingState.hxx"

namespace VulkanRHI
{

class FVulkanDevice;
class FVulkanQueue;
class VulkanCommandBufferManager;

class RVulkanTexture;

class FVulkanCommandContext : public FRHIContext, public FNamedClass
{
    RTTI_DECLARE_TYPEINFO(FVulkanCommandContext, FRHIContext);

public:
    FVulkanCommandContext(FVulkanDevice* InDevice, FVulkanQueue* InGraphicsQueue, FVulkanQueue* InPresentQueue);
    virtual ~FVulkanCommandContext();

    void Reset() override final;

public:
    /// @brief Mark the beginning of a new frame
    virtual void BeginFrame() override final;
    /// @brief Mark the end of the current frame
    virtual void EndFrame() override final;

    void SetName(std::string_view InName) override final;

    /// @brief Indicate the RHI that we are starting drawing in the given viewport
    virtual void RHIBeginDrawingViewport(RRHIViewport* const Viewport) override final;
    /// @brief Indicate the RHI that we are done drawing in the given viewport
    virtual void RHIEndDrawningViewport(RRHIViewport* const Viewport) override final;

    virtual void RHIBeginRendering(const FRHIRenderPassDescription& Description) override final;
    virtual void RHIEndRendering() override final;

    virtual void SetPipeline(Ref<RRHIGraphicsPipeline>& Pipeline) override final;
    virtual void SetMaterial(Ref<RRHIMaterial>& Material) override final;

    virtual void SetVertexBuffer(Ref<RRHIBuffer>& VertexBuffer, uint32 BufferIndex, uint32 Offset) override final;

    virtual void SetViewport(FVector3 Min, FVector3 Max) override final;
    virtual void SetScissor(IVector2 Offset, UVector2 Size) override final;

    virtual void BeginGPURegion(const std::string& Name, const FColor& Color = {}) override final;
    virtual void EndGPURegion() override final;

    virtual void Draw(uint32 BaseVertexIndex, uint32 NumPrimitives, uint32 NumInstances) override final;
    virtual void DrawIndexed(Ref<RRHIBuffer> InIndexBuffer, int32 BaseVertexIndex, uint32 FirstInstance,
                             uint32 NumVertices, uint32 StartIndex, uint32 NumPrimitives,
                             uint32 NumInstances) override final;

    virtual void CopyResourceArrayToBuffer(const IResourceArrayInterface* Source, Ref<RRHIBuffer>& Destination,
                                           uint64 SourceOffset, uint64 DestinationOffset, uint64 Size) override final;
    virtual void CopyBufferToBuffer(const Ref<RRHIBuffer>& Source, Ref<RRHIBuffer>& Destination, uint64 SourceOffset,
                                    uint64 DestinationOffset, uint64 Size) override final;

    virtual void CopyBufferToImage(const Ref<RRHIBuffer>& Source, Ref<RRHITexture>& Description, uint64 SourceOffset,
                                   IVector3 DestinationOffset, UVector3 Size) override final;

    /// @brief VulkanRHI only, set the layout of the given texture
    void SetLayout(RVulkanTexture* const Texture, VkImageLayout Layout);

    template <typename T>
    requires std::is_standard_layout_v<T>
    void SetPushConstants(const T& Data)
    {
        PendingState->SetPushConstant(Data);
    }

    VulkanCommandBufferManager* GetCommandManager() const
    {
        return CommandManager.get();
    }

    FVulkanPendingState* GetPendingState() const
    {
        return PendingState.get();
    }

private:
    std::unique_ptr<FVulkanPendingState> PendingState;
    std::unique_ptr<VulkanCommandBufferManager> CommandManager;

    FVulkanDevice* const Device = nullptr;
    FVulkanQueue* const GfxQueue = nullptr;
    FVulkanQueue* const PresentQueue = nullptr;
};

}    // namespace VulkanRHI
