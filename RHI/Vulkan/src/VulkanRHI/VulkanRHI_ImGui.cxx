#include "VulkanRHI/VulkanRHI_ImGui.hxx"

#include "Engine/Core/Application.hxx"
#include "VulkanRHI/Resources/VulkanBuffer.hxx"
#include "VulkanRHI/Resources/VulkanGraphicsPipeline.hxx"
#include "VulkanRHI/Resources/VulkanTexture.hxx"
#include "VulkanRHI/VulkanCommandsObjects.hxx"
#include "VulkanRHI/VulkanDevice.hxx"

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
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;        // Enable Docking
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;      // Enable Multi-Viewport / Platform Windows
    io.BackendRendererName = "Raphael ImGui Render";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;    // We can honor the ImDrawCmd::VtxOffset field,
                                                                  // allowing for large meshes.

    ImGui_ImplGlfw_InitForVulkan(GApplication->GetMainWindow()->GetHandle(), true);

    CreateDescriptorPool(Device);
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
                            {.Name = "Color", .Type = EVertexElementType::Float4},
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
                .ColorFormats = {EImageFormat::R8G8B8A8_SRGB},
                .DepthFormat = std::nullopt,
                .StencilFormat = std::nullopt,
            },
    });
    ensure(ImGuiPipeline);
    ImGuiPipeline->SetName("ImGui Pipeline");

    ENQUEUE_RENDER_COMMAND(ImGuiUpdateFontTexture)(
        [this](FFRHICommandList& CommandList)
        {
            ImGuiIO& io = ImGui::GetIO();

            // Build the font texture
            io.Fonts->AddFontDefault();
            io.Fonts->Build();
            UpdateFontTexture(CommandList);
        });
}

void VulkanRHI_ImGui::Shutdown()
{
    ImGui_ImplGlfw_Shutdown();

    VulkanAPI::vkDestroyDescriptorPool(Device->GetHandle(), DescriptorPool, VULKAN_CPU_ALLOCATOR);
    DescriptorPool = VK_NULL_HANDLE;

    ImGuiFontTexture = nullptr;

    ImGuiVertexBufferData.Clear();
    ImGuiVertexBuffer = nullptr;
    ImGuiIndexBufferData.Clear();
    ImGuiIndexBuffer = nullptr;
    ImGuiOutputTexture = nullptr;

    ImGuiPipeline = nullptr;
    ImGui::DestroyContext();
}

void VulkanRHI_ImGui::Begin()
{
    ENQUEUE_RENDER_COMMAND(ImGui_BeginFrame)
    (
        [](FFRHICommandList&)
        {
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // ImGui::ShowMetricsWindow();
        });
}

void VulkanRHI_ImGui::End()
{
    ENQUEUE_RENDER_COMMAND(ImGui_BeginFrame)
    ([](FFRHICommandList&) { ImGui::EndFrame(); });
}

void VulkanRHI_ImGui::Render()
{

    ENQUEUE_RENDER_COMMAND(ImGuiRenderCommand)
    (
        [this](FFRHICommandList& CommandList)
        {
            ImGui::Render();
            RenderImGuiViewport(ImGui::GetMainViewport(), CommandList);
        });
}

// #TODO magic number
constexpr auto ImGuiVertexBufferSlack = 500;

template <>
bool VulkanRHI_ImGui::ReallocateBufferIfNeeded(Ref<RVulkanBuffer>& Buffer, TResourceArray<ImDrawVert>& Data)
{
    if (Buffer && Buffer->GetCurrentSize() >= Data.GetByteSize())
    {
        return true;
    }
    // Reallocate the buffer
    FRHIBufferDesc Desc{
        .Size = Data.GetByteSize() + ImGuiVertexBufferSlack,
        .Usage = EBufferUsageFlags::SourceCopy | EBufferUsageFlags::KeepCPUAccessible,
        .ResourceArray = &Data,
        .DebugName = "ImGui Vertex Buffer Staging",
    };
    Ref<RRHIBuffer> NewStagingBuffer = RHI::CreateBuffer(Desc);

    Desc.Usage = EBufferUsageFlags::VertexBuffer | EBufferUsageFlags::DestinationCopy;
    Desc.DebugName = "ImGui Vertex Buffer";
    Desc.ResourceArray = nullptr;
    Buffer = RHI::CreateBuffer(Desc);

    ENQUEUE_RENDER_COMMAND(UpdateImGuiVertexBuffer)(
        [NewStagingBuffer, TargetBuffer = Ref<RRHIBuffer>(Buffer), Desc](FFRHICommandList& CommandList) mutable
        { CommandList.CopyBufferToBuffer(NewStagingBuffer, TargetBuffer, 0, 0, Desc.Size); });

    return true;
}
template <>
bool VulkanRHI_ImGui::ReallocateBufferIfNeeded(Ref<RVulkanBuffer>& Buffer, TResourceArray<ImDrawIdx>& Data)
{
    if (Buffer && Buffer->GetCurrentSize() >= Data.GetByteSize())
    {
        return true;
    }
    // Reallocate the buffer
    FRHIBufferDesc Desc{
        .Size = Data.GetByteSize() + ImGuiVertexBufferSlack,
        .Usage = EBufferUsageFlags::SourceCopy | EBufferUsageFlags::KeepCPUAccessible,
        .ResourceArray = &Data,
        .DebugName = "ImGui Index Buffer Staging",
    };
    Ref<RRHIBuffer> NewStagingBuffer = RHI::CreateBuffer(Desc);

    Desc.Usage = EBufferUsageFlags::IndexBuffer | EBufferUsageFlags::DestinationCopy;
    Desc.DebugName = "ImGui Index Buffer";
    Desc.ResourceArray = nullptr;
    Buffer = RHI::CreateBuffer(Desc);

    ENQUEUE_RENDER_COMMAND(UpdateImGuiIndexBuffer)(
        [NewStagingBuffer, TargetBuffer = Ref<RRHIBuffer>(Buffer), Desc](FFRHICommandList& CommandList) mutable
        { CommandList.CopyBufferToBuffer(NewStagingBuffer, TargetBuffer, 0, 0, Desc.Size); });

    return true;
}

bool VulkanRHI_ImGui::UpdateFontTexture(FFRHICommandList& CommandList)
{
    ImGuiIO& io = ImGui::GetIO();

    if (ImGuiFontTexture && io.Fonts->IsBuilt())
        return true;

    TResourceArray<uint8_t> TexturePixels;
    uint8_t* Pixels = nullptr;
    int TextureWidth = 0;
    int TextureHeight = 0;

    io.Fonts->GetTexDataAsRGBA32(&Pixels, &TextureWidth, &TextureHeight);
    if (!Pixels)
        return false;
    // #TODO: magic number
    TexturePixels.Resize(static_cast<uint32>(TextureWidth * TextureHeight * 4));
    std::memcpy(TexturePixels.Raw(), Pixels, TextureWidth * TextureHeight * 4);

    FRHITextureSpecification TextureDesc{
        .Flags = ETextureUsageFlags::SampleTargetable | ETextureUsageFlags::TransferTargetable,
        .Dimension = EImageDimension::Texture2D,
        .Format = EImageFormat::R8G8B8A8_SRGB,
        .Extent = {static_cast<uint32>(TextureWidth), static_cast<uint32>(TextureHeight)},
        .Name = "ImGui Font Texture",
    };
    ImGuiFontTexture = RHI::CreateTexture(TextureDesc);

    FRHIBufferDesc StagingBufferDesc{
        .Size = TexturePixels.GetByteSize(),
        .Usage = EBufferUsageFlags::SourceCopy | EBufferUsageFlags::KeepCPUAccessible,
        .ResourceArray = &TexturePixels,
        .DebugName = "ImGui Font Texture Staging",
    };
    Ref<RRHIBuffer> StagingBuffer = RHI::CreateBuffer(StagingBufferDesc);

    CommandList.CopyBufferToImage(StagingBuffer, ImGuiFontTexture.As<RRHITexture>(), 0, {0, 0, 0},
                                  {static_cast<uint32>(TextureWidth), static_cast<uint32>(TextureHeight), 1});

    io.Fonts->TexID = (ImTextureID)ImGuiFontTexture.Raw();
    return true;
}

bool VulkanRHI_ImGui::RenderImGuiViewport(ImGuiViewport* Viewport, FFRHICommandList& CommandList)
{
    if (!UpdateTargetTexture(Viewport, CommandList))
    {
        return false;
    }

    ImDrawData* const DrawData = ImGui::GetDrawData();
    // Nothing to draw, yay
    if (DrawData->TotalVtxCount == 0)
    {
        return true;
    }

    if (!UpdateGeometry(DrawData))
        return false;

    FVulkanCommandContext* CommandContext = CommandList.GetContext()->Cast<FVulkanCommandContext>();

    if (!UpdateFontTexture(CommandList))
        return false;

    CommandList.BeginFrame();

    FRHIRenderPassDescription RenderPassDesc{
        .RenderAreaLocation = {0, 0},
        .RenderAreaSize = {static_cast<uint32>(Viewport->Size.x), static_cast<uint32>(Viewport->Size.y)},
        .ColorTargets =
            {
                {
                    .Texture = ImGuiOutputTexture,
                    .ClearColor = {0.0f, 0.0f, 0.0f, 1.0f},
                    .LoadAction = ERenderTargetLoadAction::Clear,
                    .StoreAction = ERenderTargetStoreAction::Store,
                },
            },
    };
    CommandList.BeginRendering(RenderPassDesc);

    CommandList.SetVertexBuffer(ImGuiVertexBuffer);
    CommandList.SetGraphicsPipeline(ImGuiPipeline);

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
                // drawState.bindings = {GetBindingSet((nvrhi::ITexture*)pCmd->TextureId)};
                // HZ_CORE_ASSERT(drawState.bindings[0]);

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
                    clipMax.x = (float)fbWidth;
                if (clipMax.y > fbHeight)
                    clipMax.y = (float)fbHeight;
                if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y)
                    continue;

                CommandList.SetViewport({clipMin.x, clipMin.y, 0}, {clipMax.x, clipMax.y, 1});

                int vertexCount = pCmd->ElemCount;
                int startIndexLocation = pCmd->IdxOffset + idxOffset;
                int startVertexLocation = pCmd->VtxOffset + vtxOffset;

                CommandContext->SetPushConstants(pushConstants);
                CommandList.DrawIndexed(ImGuiIndexBuffer, 0, 0, vertexCount, startIndexLocation, startVertexLocation,
                                        1);
            }
        }

        vtxOffset += cmdList->VtxBuffer.Size;
        idxOffset += cmdList->IdxBuffer.Size;
    }

    CommandList.EndRendering();
    CommandList.EndFrame();

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

    if (!ReallocateBufferIfNeeded(ImGuiVertexBuffer, ImGuiVertexBufferData))
        return false;
    if (!ReallocateBufferIfNeeded(ImGuiIndexBuffer, ImGuiIndexBufferData))
        return false;

    return true;
}

bool VulkanRHI_ImGui::UpdateTargetTexture(ImGuiViewport* Viewport, FFRHICommandList& CommandList)
{
    if (ImGuiOutputTexture)
    {
        if (ImGuiOutputTexture->GetDescription().Extent.x == Viewport->Size.x ||
            ImGuiOutputTexture->GetDescription().Extent.y == Viewport->Size.y)
        {
            return true;
        }
    }

    FRHITextureSpecification TextureDesc{
        .Flags = ETextureUsageFlags::RenderTargetable | ETextureUsageFlags::TransferTargetable,
        .Dimension = EImageDimension::Texture2D,
        .Format = EImageFormat::R8G8B8A8_SRGB,
        .Extent = {static_cast<uint32>(Viewport->Size.x), static_cast<uint32>(Viewport->Size.y)},
        .Name = "ImGui Output Texture",
    };
    ImGuiOutputTexture = RHI::CreateTexture(TextureDesc);

    FVulkanCommandContext* Context = CommandList.GetContext()->Cast<FVulkanCommandContext>();
    Ref<RVulkanTexture> OutputTexture = ImGuiOutputTexture.As<RVulkanTexture>();
    OutputTexture->SetLayout(Context->GetCommandManager()->GetUploadCmdBuffer(),
                             VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    return true;
}

void VulkanRHI_ImGui::CreateDescriptorPool(FVulkanDevice* Device)
{
    const VkDescriptorPoolSize pool_sizes[]{
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000},
    };

    VkDescriptorPoolCreateInfo ImguiPoolCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = 1000,
        .poolSizeCount = std::size(pool_sizes),
        .pPoolSizes = pool_sizes,
    };
    VulkanAPI::vkCreateDescriptorPool(Device->GetHandle(), &ImguiPoolCreateInfo, VULKAN_CPU_ALLOCATOR, &DescriptorPool);
    VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_DESCRIPTOR_POOL, DescriptorPool, "Imgui Descriptor Pool");
}

}    // namespace VulkanRHI
