#include "VulkanRHI/VulkanRHI_ImGui.hxx"

#include "Engine/Core/Application.hxx"
#include "VulkanRHI/VulkanCommandsObjects.hxx"
#include "VulkanRHI/VulkanDevice.hxx"

#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

namespace VulkanRHI
{

void VulkanRHI_ImGui::Initialize(FVulkanDevice* Device, ImGui_ImplVulkan_InitInfo InInitInfo)
{

    IMGUI_CHECKVERSION();
    ImGui_ImplVulkan_LoadFunctions(
        RHI_VULKAN_VERSION,
        [](const char* FunctionName, void* UserData) -> PFN_vkVoidFunction
        {
            VkInstance Instance = reinterpret_cast<VkInstance>(UserData);
            return VulkanAPI::vkGetInstanceProcAddr(Instance, FunctionName);
        },
        InInitInfo.Instance);

    InitInfo = InInitInfo;
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;    // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;        // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;      // Enable Multi-Viewport / Platform Windows

    CreateDescriptorPool(Device);

    InitInfo.DescriptorPool = DescriptorPool;
    ImGui_ImplGlfw_InitForVulkan(GApplication->GetMainWindow()->GetHandle(), true);
    ImGui_ImplVulkan_Init(&InitInfo);

    // ENQUEUE_RENDER_COMMAND(UploadImGuiFont)([](FFRHICommandList&) { ImGui_ImplVulkan_CreateFontsTexture(); });
}

void VulkanRHI_ImGui::Shutdown()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    VulkanAPI::vkDestroyDescriptorPool(InitInfo.Device, DescriptorPool, VULKAN_CPU_ALLOCATOR);
    DescriptorPool = VK_NULL_HANDLE;
    ImGui::DestroyContext();
}

void VulkanRHI_ImGui::Begin()
{
    ENQUEUE_RENDER_COMMAND(ImGui_BeginFrame)
    (
        [](FFRHICommandList&)
        {
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::ShowMetricsWindow();
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
        [](FFRHICommandList& CommandList)
        {
            ImGui::Render();
            FVulkanCommandContext* CommandContext = CommandList.GetContext()->Cast<FVulkanCommandContext>();

            CommandContext->GetCommandManager()->PrepareForNewActiveCommandBuffer();
            FVulkanCmdBuffer* CmdBuffer = CommandContext->GetCommandManager()->GetActiveCmdBuffer();

            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), CmdBuffer->GetHandle());

            ImGuiIO& io = ImGui::GetIO();
            // Update and Render additional Platform Windows
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
            }
        });
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
