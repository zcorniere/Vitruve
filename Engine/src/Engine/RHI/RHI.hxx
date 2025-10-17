#pragma once

#include "Engine/RHI/RHIResource.hxx"
#include <Engine/RHI/RHICommandList.hxx>

#include <future>

class RWindow;

DECLARE_LOGGER_CATEGORY(Core, LogRHI, Info);

enum class ERHIInterfaceType
{
    Null,
    Vulkan,
};

extern ENGINE_API class FGenericRHI* GDynamicRHI;

/// Wrapper function around the RHI function
namespace RHI
{

/// @brief Return the current RHI
/// @tparam TRHI The type of the RHI, default is GenericRHI
template <typename TRHI = FGenericRHI>
FORCEINLINE TRHI* Get()
{
    checkMsg(GDynamicRHI, "Attemped to fetch the RHI to early !");
    return static_cast<TRHI*>(GDynamicRHI);
}

/// @brief This function create the RHI, and perform early initialisation
ENGINE_API void Create();

/// @brief Called every frame from the main loop
ENGINE_API void Tick(float fDeltaTime);

/// @brief Delete the current RHI
ENGINE_API void Destroy();

/// @brief Defer the execution of the given function to the next frame
/// @param InDeletionFunction The function to defer
///
/// This function is used to defer the deletion of resources to the next frame, this is useful when the resource is in
/// use and cannot be deleted immediately
ENGINE_API void DeferedDeletion(std::function<void()>&& InDeletionFunction);
ENGINE_API void FlushDeletionQueue(bool bAsync);

/// -------------- RHI Operations --------------

/// @brief Mark the beginning of a new frame
ENGINE_API void BeginFrame();
/// @brief Mark the end of the current frame
ENGINE_API void EndFrame();

ENGINE_API void RHIWaitUntilIdle();

/// Create a new RHI viewport - through the current RHI
ENGINE_API Ref<RRHIViewport> CreateViewport(Ref<RWindow> InWindowHandle, UVector2 InSize, bool bCreateDepthBuffer);
/// Create a new RHI texture - through the current RHI
ENGINE_API Ref<RRHITexture> CreateTexture(const FRHITextureSpecification& InDesc);
/// Create a new RHI buffer - through the current RHI
ENGINE_API Ref<RRHIBuffer> CreateBuffer(const FRHIBufferDesc& InDesc);
/// Create a new RHI shader - through the current RHI
ENGINE_API Ref<RRHIShader> CreateShader(const std::filesystem::path Path, bool bForceCompile);
/// Create a new RHI shader - through the current RHI asynchronously
ENGINE_API std::future<Ref<RRHIShader>> CreateShaderAsync(const std::filesystem::path Path, bool bForceCompile);
/// Create a new RHI Pipeline - through the current RHI
ENGINE_API Ref<RRHIGraphicsPipeline> CreateGraphicsPipeline(const FRHIGraphicsPipelineSpecification& Config);
/// Create a new RHI Material - through the current RHI
ENGINE_API Ref<RRHIMaterial> CreateMaterial(const WeakRef<RRHIGraphicsPipeline>& Pipeline);

};    // namespace RHI
