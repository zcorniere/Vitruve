#include "Engine/Core/RHI/RHI.hxx"

#include "Engine/Core/Engine.hxx"
#include "Engine/Core/RHI/GenericRHI.hxx"
#include "Engine/Core/RHI/RHICommandList.hxx"
#include "Engine/Core/Window.hxx"

#include "AssetRegistry/AssetRegistry.hxx"

ENGINE_API FGenericRHI* GDynamicRHI = nullptr;

static IExternalModule* s_RHIModuleHandle = nullptr;

void RHI::Create()
{
    const char* VulkanRHIName = "VitruveRHIVulkan";
    std::string ModulePath =
        std::format("{:s}/RHI/Vulkan/{:s}", std::filesystem::current_path().string(), VITRUVE_BUILD_TYPE);

#if defined(PLATFORM_WINDOWS)
    s_RHIModuleHandle = FPlatformMisc::LoadExternalModule(std::format("{}/{:s}.dll", ModulePath, VulkanRHIName));
#elif defined(PLATFORM_LINUX)
    s_RHIModuleHandle = FPlatformMisc::LoadExternalModule(std::format("{}/lib{:s}.so", ModulePath, VulkanRHIName));
#endif

    check(s_RHIModuleHandle->IsValid());
    FGenericRHI* (*CreateRHI_Funct)() = s_RHIModuleHandle->GetSymbol<FGenericRHI* (*)()>("RHI_CreateRHI");
    GDynamicRHI = CreateRHI_Funct();
}

void RHI::Destroy()
{
    GEngine->AssetRegistry->Purge();

    RHI::FlushDeletionQueue(false);
    GDynamicRHI->Shutdown();

    delete GDynamicRHI;
    GDynamicRHI = nullptr;
    delete s_RHIModuleHandle;
    s_RHIModuleHandle = nullptr;
}

void RHI::BeginFrame()
{
}

void RHI::Tick(float fDeltaTime)
{
    return RHI::Get()->Tick(fDeltaTime);
}

void RHI::EndFrame()
{
    // Run the command list
    RHI::Get()->PostFrame();

    FRHIContext* const Context = RHI::Get()->RHIGetCommandContext();
    FRHICommandListExecutor::Get().GetCommandList().Execute(Context);
    RHI::Get()->RHIReleaseCommandContext(Context);

    GFrameCounter += 1;
}

void RHI::DeferedDeletion(std::function<void()>&& InDeletionFunction)
{
    RHI::Get()->DeferedDeletion(std::move(InDeletionFunction));
}

void RHI::FlushDeletionQueue(bool bAsync)
{
    if (bAsync)
    {
        RHI::Get()->FlushDeletionQueueAsync();
    }
    else
    {
        RHI::Get()->FlushDeletionQueue();
    }
}

void RHI::RHIWaitUntilIdle()
{
    RHI::Get()->WaitUntilIdle();
}

//
//  -------------------- RHI Create resources --------------------
//

Ref<RRHIViewport> RHI::CreateViewport(Ref<RWindow> InWindowHandle, UVector2 InSize, bool bCreateDepthBuffer)
{
    return RHI::Get()->CreateViewport(std::move(InWindowHandle), std::move(InSize), bCreateDepthBuffer);
}

Ref<RRHITexture> RHI::CreateTexture(const FRHITextureSpecification& InDesc)
{
    return RHI::Get()->CreateTexture(InDesc);
}

Ref<RRHIBuffer> RHI::CreateBuffer(const FRHIBufferDesc& InDesc)
{
    return RHI::Get()->CreateBuffer(InDesc);
}

Ref<RRHIShader> RHI::CreateShader(const std::filesystem::path Path, bool bForceCompile)
{
    return RHI::Get()->CreateShader(Path, bForceCompile);
}

std::future<Ref<RRHIShader>> RHI::CreateShaderAsync(const std::filesystem::path Path, bool bForceCompile)
{
    return GEngine->GetThreadPool().Push([Path, bForceCompile](int)
                                         { return RHI::Get()->CreateShader(Path, bForceCompile); });
}

Ref<RRHIGraphicsPipeline> RHI::CreateGraphicsPipeline(const FRHIGraphicsPipelineSpecification& Config)
{
    return RHI::Get()->CreateGraphicsPipeline(Config);
}

Ref<RRHIMaterial> RHI::CreateMaterial(const WeakRef<RRHIGraphicsPipeline>& Pipeline)
{
    return RHI::Get()->CreateMaterial(Pipeline);
}
