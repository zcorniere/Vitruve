#pragma once

#include "Engine/Containers/ResourceArray.hxx"
#include "Engine/Core/RHI/RHICommandList.hxx"
#include "Engine/Core/Window.hxx"
#include "VulkanRHI/DescriptorPoolManager.hxx"

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

    void BeginFrame(FFRHICommandList& CommandList);
    void EndFrame(FFRHICommandList& CommandList);
    void Render(FFRHICommandList& CommandList);

    Ref<RVulkanTexture> GetOutputTexture() const
    {
        return ImGuiOutputTexture;
    }

private:
    bool UpdateFontTexture(FFRHICommandList& CommandList);
    bool RenderImGuiViewport(ImGuiViewport* Viewport, FFRHICommandList& CommandList);
    bool UpdateGeometry(ImDrawData* DrawData);
    bool UpdateTargetTexture(ImGuiViewport* Viewport, FFRHICommandList& CommandList);

private:
    GLFWHolder GlfwHolder;
    FVulkanDevice* Device = nullptr;

    Ref<RVulkanTexture> ImGuiFontTexture = nullptr;

    TResourceArray<ImDrawVert> ImGuiVertexBufferData;
    Ref<RVulkanBuffer> ImGuiVertexBuffer = nullptr;

    TResourceArray<ImDrawIdx> ImGuiIndexBufferData;
    Ref<RVulkanBuffer> ImGuiIndexBuffer = nullptr;
    Ref<RVulkanGraphicsPipeline> ImGuiPipeline = nullptr;
    std::unique_ptr<FDescriptorSetManager> DescriptorSetManager;

    Ref<RVulkanTexture> ImGuiOutputTexture = nullptr;
};

}    // namespace VulkanRHI
