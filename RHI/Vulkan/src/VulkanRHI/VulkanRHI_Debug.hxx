#pragma once

class FFRHICommandList;

namespace VulkanRHI
{

class FVulkanRegion
{
public:
    FVulkanRegion(FFRHICommandList& CommandList, const std::string& Name, const FColor& Color = {});
    ~FVulkanRegion();

private:
    FFRHICommandList& CommandList;
};

#if VULKAN_DEBUGGING_ENABLED

class VulkanRHI_Debug
{
public:
    TArray<const char*> GetSupportedInstanceLayers();
    void SetupDebugLayer(VkInstance Instance);
    void RemoveDebugLayer(VkInstance Instance);

    bool IsValidationLayersMissing() const
    {
        return bValidationLayersAreMissing;
    }

private:
    bool bValidationLayersAreMissing = false;
    VkDebugUtilsMessengerEXT Messenger = VK_NULL_HANDLE;
};

#endif    // VULKAN_DEBUGGING_ENABLED

}    // namespace VulkanRHI
