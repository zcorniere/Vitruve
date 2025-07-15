#pragma once

#include "Engine/Containers/ResourceArray.hxx"

#include "Engine/Core/RHI/RHICommandList.hxx"

#include "imgui.h"

class RWindow;

namespace VulkanRHI
{
class FVulkanDevice;

class RVulkanGraphicsPipeline;
class RVulkanTexture;
class RVulkanBuffer;

class VulkanRHI_ImGui
{
public:
    void Initialize(FVulkanDevice* Device);
    void Shutdown();

    void Begin();
    void End();
    void Render();

private:
    bool UpdateFontTexture(FFRHICommandList& CommandList);
    bool RenderImGuiViewport(ImGuiViewport* Viewport, FFRHICommandList& CommandList);
    bool UpdateGeometry(ImDrawData* DrawData);
    bool UpdateTargetTexture(ImGuiViewport* Viewport, FFRHICommandList& CommandList);

    template <typename T>
    bool ReallocateBufferIfNeeded(Ref<RVulkanBuffer>& Buffer, TResourceArray<T>& Data);

    void CreateDescriptorPool(FVulkanDevice* Device);

private:
    FVulkanDevice* Device = nullptr;
    VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
    Ref<RVulkanTexture> ImGuiFontTexture = nullptr;

    Ref<RVulkanTexture> FontTexture = nullptr;
    VkSampler FontSampler = VK_NULL_HANDLE;

    TResourceArray<ImDrawVert> ImGuiVertexBufferData;
    Ref<RVulkanBuffer> ImGuiVertexBuffer = nullptr;

    TResourceArray<ImDrawIdx> ImGuiIndexBufferData;
    Ref<RVulkanBuffer> ImGuiIndexBuffer = nullptr;
    Ref<RVulkanGraphicsPipeline> ImGuiPipeline = nullptr;

    Ref<RVulkanTexture> ImGuiOutputTexture = nullptr;
};

}    // namespace VulkanRHI
