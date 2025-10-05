#include "Engine/RHI/RHI.hxx"

#include "Engine/Core/Engine.hxx"
#include "Engine/Core/Window.hxx"
#include "Engine/RHI/GenericRHI.hxx"
#include "Engine/RHI/RHICommandList.hxx"

#include "AssetRegistry/AssetRegistry.hxx"
#include "Engine/Modules/ModuleInterface.hxx"
#include "Engine/Modules/ModuleManager.hxx"

ENGINE_API FGenericRHI* GDynamicRHI = nullptr;

void RHI::Create()
{
    FModuleManager::Get().AddDLLSearchPath(std::filesystem::current_path() / "RHI/Vulkan");
    IModuleInterface* const ModuleInterface = FModuleManager::Get().LoadModule("VulkanRHI");
    check(ModuleInterface);

    IRHIModule* const RHIModuleInterface = ModuleInterface->Cast<IRHIModule>();
    check(RHIModuleInterface);
    GDynamicRHI = RHIModuleInterface->CreateRHI();
}

void RHI::Destroy()
{
    GEngine->AssetRegistry->Purge();

    RHI::FlushDeletionQueue(false);
    GDynamicRHI->Shutdown();

    delete GDynamicRHI;
    GDynamicRHI = nullptr;

    FModuleManager::Get().UnloadModule("VulkanRHI");
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
