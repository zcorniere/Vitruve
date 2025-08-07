#include "VulkanRHI/VulkanRHI_ImGui.hxx"

#include "Engine/Core/Application.hxx"

#include "VulkanRHI/Resources/VulkanBuffer.hxx"
#include "VulkanRHI/Resources/VulkanGraphicsPipeline.hxx"
#include "VulkanRHI/Resources/VulkanTexture.hxx"
#include "VulkanRHI/VulkanCommandsObjects.hxx"
#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanMemoryManager.hxx"

#include "imgui_impl_glfw.h"

namespace VulkanRHI
{

void VulkanRHI_ImGui::Initialize(FVulkanDevice* Device)
{
    this->Device = Device;

    IMGUI_CHECKVERSION();

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;    // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;    // Enable Docking
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;      // Enable Multi-Viewport / Platform Windows

    io.BackendRendererName = "Raphael ImGui Render";
    // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
    // We can honor ImGuiPlatformIO::Textures[] requests during render.
    io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;

    ImGui_ImplGlfw_InitForVulkan(GApplication->GetMainWindow()->GetHandle(), true);

    ImGuiPipeline = RHI::CreateGraphicsPipeline(FRHIGraphicsPipelineSpecification{
        .VertexShader = "ImGui/ImGui.vert",
        .FragmentShader = "ImGui/ImGui.frag",
        .VertexBufferLayouts =
            {
                {
                    .InputMode = EVertexInputMode::PerVertex,
                    .Parameter =
                        {
                            {.Name = "Position", .Type = EVertexElementType::Float2},
                            {.Name = "Texcoord", .Type = EVertexElementType::Float2},
                            // {.Name = "Color", .Type = EVertexElementType::Float4},
                            {.Name = "Color", .Type = EVertexElementType::Uint1},
                        },
                },
            },
        .Rasterizer =
            {
                .PolygonMode = EPolygonMode::Fill,
                .CullMode = ECullMode::None,
                .FrontFaceCulling = EFrontFace::CounterClockwise,
            },
        .AttachmentFormats =
            {
                .ColorFormats = {EImageFormat::B8G8R8A8_SRGB},
                .DepthFormat = std::nullopt,
                .StencilFormat = std::nullopt,
            },
    });
    ensure(ImGuiPipeline);
    ImGuiPipeline->SetName("ImGui Pipeline");

    DescriptorSetManager = std::make_unique<FDescriptorSetManager>(Device, ImGuiPipeline);
    DescriptorSetManager->Bake();
}

void VulkanRHI_ImGui::Shutdown()
{
    ImGui_ImplGlfw_Shutdown();

    ImGuiVertexBufferData.Clear();
    ImGuiVertexBuffer = nullptr;
    ImGuiIndexBufferData.Clear();
    ImGuiIndexBuffer = nullptr;

    DescriptorSetManager.reset();

    for (ImTextureData* tex: ImGui::GetPlatformIO().Textures)
    {
        tex->SetTexID(0);
    }
    ImGuiTexturesArray.Clear();

    ImGuiPipeline = nullptr;
    ImGui::DestroyContext();
}

void VulkanRHI_ImGui::BeginFrame(FFRHICommandList&)
{
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowMetricsWindow();
}

void VulkanRHI_ImGui::EndFrame(FFRHICommandList&)
{
    ImGui::EndFrame();
}

void VulkanRHI_ImGui::Render(FFRHICommandList& CommandList, RRHIViewport* BackBuffer)
{
    RPH_PROFILE_FUNC()
    ImGui::Render();

    {
        const ImVec4& Color = ImGui::GetStyleColorVec4(ImGuiCol_TitleBg);
        FVulkanRegion Region(CommandList, "ImGui Render", *(reinterpret_cast<const FColor*>(&Color)));
        RenderImGuiViewport(ImGui::GetMainViewport(), CommandList, BackBuffer);
    }
}

template <typename T, EBufferUsageFlags Flags, int BufferSlack = 500>
static bool ReallocateBufferIfNeeded(Ref<RVulkanBuffer>& Buffer, TResourceArray<T>& Data)
{
    if (Buffer && Buffer->GetCurrentSize() >= Data.GetByteSize())
    {
        return false;
    }
    // Reallocate the buffer
    FRHIBufferDesc Desc{
        .Size = Data.GetByteSize() + BufferSlack,
        .Stride = sizeof(T),
        .Usage = Flags | EBufferUsageFlags::KeepCPUAccessible,
        .ResourceArray = &Data,
        .DebugName = "ImGuiBufferStaging",
    };

    if constexpr (EnumHasAnyFlags(Flags, EBufferUsageFlags::VertexBuffer))
    {
        Desc.DebugName = "ImGuiBuffer.Vertex";
    }
    else if constexpr (EnumHasAnyFlags(Flags, EBufferUsageFlags::IndexBuffer))
    {
        Desc.DebugName = "ImGuiBuffer.Index";
    }
    else
    {
        Desc.DebugName = "ImGuiBuffer";
    }
    Buffer = RHI::CreateBuffer(Desc);

    return true;
}

bool VulkanRHI_ImGui::UpdateTexture(ImTextureData* Texture, FFRHICommandList& CommandList)
{
    check(Texture);
    check(Texture->GetTexID() == 0 || Texture->GetTexID() <= ImGuiTexturesArray.Size());

    Ref<RVulkanTexture> VulkanTexture;
    if (Texture->Status == ImTextureStatus_WantCreate)
    {
        const FRHITextureSpecification TextureDesc{
            .Flags = ETextureUsageFlags::SampleTargetable | ETextureUsageFlags::TransferTargetable,
            .Dimension = EImageDimension::Texture2D,
            .Format = EImageFormat::R8G8B8A8_SRGB,
            .Extent = {static_cast<uint32>(Texture->Width), static_cast<uint32>(Texture->Height)},
            .NumSamples = 1,
            .Name = std::format("ImGuiTexture.{}", ImGuiTexturesArray.Size()),
        };
        ImGuiTexturesArray.Emplace(RHI::CreateTexture(TextureDesc));
        // The actual index is "TexID - 1", because we need to reserve value 0 for "nothing"
        Texture->SetTexID(ImGuiTexturesArray.Size());
        VulkanTexture = ImGuiTexturesArray.Back();
    }
    else
    {
        VulkanTexture = ImGuiTexturesArray[Texture->GetTexID() - 1];
    }

    if (Texture->Status == ImTextureStatus_WantCreate || Texture->Status == ImTextureStatus_WantUpdates)
    {
        const int32 UploadX = (Texture->Status == ImTextureStatus_WantCreate) ? 0 : Texture->UpdateRect.x;
        const int32 UploadY = (Texture->Status == ImTextureStatus_WantCreate) ? 0 : Texture->UpdateRect.y;
        const uint32 UploadW = (Texture->Status == ImTextureStatus_WantCreate) ? Texture->Width : Texture->UpdateRect.w;
        const uint32 UploadH =
            (Texture->Status == ImTextureStatus_WantCreate) ? Texture->Height : Texture->UpdateRect.h;
        const uint32 UploadPitch = UploadW * Texture->BytesPerPixel;
        const uint32 UploadSize = UploadH * UploadPitch;

        const FRHIBufferDesc Description{
            .Size = UploadSize,
            .Stride = static_cast<uint32>(Texture->BytesPerPixel),
            .Usage = EBufferUsageFlags::SourceCopy | EBufferUsageFlags::KeepCPUAccessible,
            .DebugName = std::format("{}.StagingBuffer", VulkanTexture->GetName()),
        };
        Ref<RVulkanBuffer> StagingBuffer = RHI::CreateBuffer(Description);
        uint8* BufferMemory = (uint8*)StagingBuffer->GetMemory()->Map(UploadSize);
        for (uint32 y = 0; y < UploadH; y++)
        {
            std::memcpy(BufferMemory + UploadPitch * y, Texture->GetPixelsAt(UploadX, UploadY + y), UploadPitch);
        }
        StagingBuffer->GetMemory()->FlushMappedMemory(0, UploadSize);
        StagingBuffer->GetMemory()->Unmap();

        Device->GetImmediateContext()->SetLayout(VulkanTexture.Raw(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        Ref<RRHITexture> RHITexture = VulkanTexture.As<RRHITexture>();
        Device->GetImmediateContext()->CopyBufferToImage(StagingBuffer, RHITexture, 0, {UploadX, UploadY, 0},
                                                         {UploadW, UploadH, 1});
        Device->GetImmediateContext()->SetLayout(VulkanTexture.Raw(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        Device->GetImmediateContext()->GetCommandManager()->SubmitUploadCmdBuffer();
        Device->WaitUntilIdle();    // TODO: Not that
        Texture->Status = ImTextureStatus_OK;
    }

    if (Texture->Status == ImTextureStatus_WantDestroy)
    {
        // TODO: worry about that
        ensure(false);
    }

    return true;
}

bool VulkanRHI_ImGui::RenderImGuiViewport(ImGuiViewport* Viewport, FFRHICommandList& CommandList,
                                          RRHIViewport* RenderingViewport)
{
    RPH_PROFILE_FUNC()
    ImDrawData* const DrawData = ImGui::GetDrawData();
    // Nothing to draw, yay
    if (DrawData->TotalVtxCount == 0)
    {
        return true;
    }

    FVulkanCommandContext* CommandContext = CommandList.GetContext()->Cast<FVulkanCommandContext>();

    UpdateGeometry(DrawData);

    DrawData->ScaleClipRects(DrawData->FramebufferScale);

    if (DrawData->Textures)
    {
        for (ImTextureData* tex: *DrawData->Textures)
        {
            if (tex->Status != ImTextureStatus_OK)
            {
                UpdateTexture(tex, CommandList);
            }
        }
    }
    FRHIRenderPassDescription RenderPassDesc{
        .RenderAreaLocation = {0, 0},
        .RenderAreaSize = {static_cast<uint32>(Viewport->Size.x), static_cast<uint32>(Viewport->Size.y)},
        .ColorTargets =
            {
                {
                    .Texture = RenderingViewport->GetBackbuffer(),
                    .ClearColor = {0.0f, 0.0f, 0.0f, 1.0f},
                    .LoadAction = ERenderTargetLoadAction::Load,
                    .StoreAction = ERenderTargetStoreAction::Store,
                },
            },
    };
    CommandList.BeginRenderingViewport(RenderingViewport);
    CommandList.BeginRendering(RenderPassDesc);

    CommandList.SetGraphicsPipeline(ImGuiPipeline);
    CommandList.SetVertexBuffer(ImGuiVertexBuffer);

    struct PushConstants
    {
        FVector2 Scale;
        FVector2 Translate;
    } pushConstants;

    pushConstants.Scale.x = 2.0f / DrawData->DisplaySize.x;
    pushConstants.Scale.y = 2.0f / DrawData->DisplaySize.y;
    pushConstants.Translate.x = -1.0f - DrawData->DisplayPos.x * pushConstants.Scale.x;
    pushConstants.Translate.y = -1.0f - DrawData->DisplayPos.y * pushConstants.Scale.y;

    const float fbWidth = DrawData->DisplaySize.x * DrawData->FramebufferScale.x;
    const float fbHeight = DrawData->DisplaySize.y * DrawData->FramebufferScale.y;

    CommandList.SetViewport({0, 0, 0}, {fbWidth, fbHeight, 1.0f});

    // Will project scissor/clipping rectangles into framebuffer space
    ImVec2 clip_off = DrawData->DisplayPos;            // (0,0) unless using multi-viewports
    ImVec2 clip_scale = DrawData->FramebufferScale;    // (1,1) unless using retina display which are often (2,2)

    // render command lists
    int vtxOffset = 0;
    int idxOffset = 0;
    for (int n = 0; n < DrawData->CmdListsCount; n++)
    {
        const ImDrawList* cmdList = DrawData->CmdLists[n];
        for (int i = 0; i < cmdList->CmdBuffer.Size; i++)
        {
            const ImDrawCmd* pCmd = &cmdList->CmdBuffer[i];

            if (pCmd->UserCallback)
            {
                pCmd->UserCallback(cmdList, pCmd);
            }
            else
            {
                // Project scissor/clipping rectangles into framebuffer space
                ImVec2 clipMin((pCmd->ClipRect.x - clip_off.x) * clip_scale.x,
                               (pCmd->ClipRect.y - clip_off.y) * clip_scale.y);
                ImVec2 clipMax((pCmd->ClipRect.z - clip_off.x) * clip_scale.x,
                               (pCmd->ClipRect.w - clip_off.y) * clip_scale.y);

                // Clamp to viewport as vkCmdSetScissor() won't accept values that are off bounds
                if (clipMin.x < 0.0f)
                    clipMin.x = 0.0f;
                if (clipMin.y < 0.0f)
                    clipMin.y = 0.0f;
                if (clipMax.x > fbWidth)
                    clipMax.x = fbWidth;
                if (clipMax.y > fbHeight)
                    clipMax.y = fbHeight;
                if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y)
                    continue;

                CommandList.SetScissor({static_cast<int32>(clipMin.x), static_cast<int32>(clipMin.y)},
                                       {static_cast<uint32>(clipMax.x), static_cast<uint32>(clipMax.y)});

                int vertexCount = pCmd->ElemCount;
                int startIndexLocation = pCmd->IdxOffset + idxOffset;
                int startVertexLocation = pCmd->VtxOffset + vtxOffset;

                check(pCmd->GetTexID() == 0 || pCmd->GetTexID() <= ImGuiTexturesArray.Size());
                DescriptorSetManager->SetInput("sTexture", ImGuiTexturesArray[pCmd->GetTexID() - 1]);
                DescriptorSetManager->InvalidateAndUpdate();
                DescriptorSetManager->Bind(CommandContext->GetCommandManager()->GetActiveCmdBuffer()->GetHandle(),
                                           ImGuiPipeline->GetPipelineLayout(), VK_PIPELINE_BIND_POINT_GRAPHICS);

                CommandContext->SetPushConstants(pushConstants);
                CommandList.DrawIndexed(ImGuiIndexBuffer, startVertexLocation, 0, vertexCount, startIndexLocation,
                                        vertexCount / 3, 1);
            }
        }

        vtxOffset += cmdList->VtxBuffer.Size;
        idxOffset += cmdList->IdxBuffer.Size;
    }

    CommandList.EndRendering();

    return true;
}

bool VulkanRHI_ImGui::UpdateGeometry(ImDrawData* DrawData)
{
    ImGuiVertexBufferData.Resize(DrawData->TotalVtxCount);
    ImGuiIndexBufferData.Resize(DrawData->TotalIdxCount);

    ImDrawVert* vtxDst = &ImGuiVertexBufferData[0];
    ImDrawIdx* idxDst = &ImGuiIndexBufferData[0];

    for (int n = 0; n < DrawData->CmdListsCount; n++)
    {
        const ImDrawList* const cmdList = DrawData->CmdLists[n];
        std::memcpy(vtxDst, cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
        std::memcpy(idxDst, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));

        vtxDst += cmdList->VtxBuffer.Size;
        idxDst += cmdList->IdxBuffer.Size;
    }

    bool bWasResized = false;
    bWasResized |=
        ReallocateBufferIfNeeded<ImDrawVert, EBufferUsageFlags::VertexBuffer>(ImGuiVertexBuffer, ImGuiVertexBufferData);
    bWasResized |=
        ReallocateBufferIfNeeded<ImDrawIdx, EBufferUsageFlags::IndexBuffer>(ImGuiIndexBuffer, ImGuiIndexBufferData);

    return bWasResized;
}

}    // namespace VulkanRHI
