#include "Engine/Core/Window.hxx"
#include "GLFW/glfw3.h"

#include "VulkanRHI/VulkanExtension.hxx"
#include "VulkanRHI/VulkanPlatform.hxx"

template <typename ExistingChainType, typename NewStructType>
static void AddToPNext(ExistingChainType& Existing, NewStructType& Added)
{
    Added.pNext = (void*)Existing.pNext;
    Existing.pNext = (void*)&Added;
}

namespace VulkanRHI
{

class FDynamicRenderingExtension : public IDeviceVulkanExtension
{
public:
    FDynamicRenderingExtension(): IDeviceVulkanExtension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME, true)
    {
        std::memset(&DynamicRenderingFeature, 0, sizeof(DynamicRenderingFeature));
        DynamicRenderingFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
        DynamicRenderingFeature.dynamicRendering = VK_TRUE;
    }

    void PreDeviceCreated(VkDeviceCreateInfo& DeviceInfo) override final
    {
        AddToPNext(DeviceInfo, DynamicRenderingFeature);
    }

private:
    VkPhysicalDeviceDynamicRenderingFeatures DynamicRenderingFeature{};
};

class FMaintenance5Extension : public IDeviceVulkanExtension
{
public:
    FMaintenance5Extension(): IDeviceVulkanExtension(VK_KHR_MAINTENANCE_5_EXTENSION_NAME, false)
    {
        std::memset(&Maintenance5Feature, 0, sizeof(Maintenance5Feature));
        Maintenance5Feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES_KHR;
        Maintenance5Feature.maintenance5 = VK_TRUE;
    }

    void PreDeviceCreated(VkDeviceCreateInfo& DeviceInfo) override final
    {
        AddToPNext(DeviceInfo, Maintenance5Feature);
    }

    void PostDeviceCreated(FOptionalExtensionStatus& Status) override final
    {
        Status.Maintenance5 = IsSupported();
    }

private:
    VkPhysicalDeviceMaintenance5FeaturesKHR Maintenance5Feature{};
};

class FSynchronisation2Extension : public IDeviceVulkanExtension
{
public:
    FSynchronisation2Extension(): IDeviceVulkanExtension(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, true)
    {
        std::memset(&Synchronisation2Feature, 0, sizeof(Synchronisation2Feature));
        Synchronisation2Feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
        Synchronisation2Feature.synchronization2 = VK_TRUE;
    }

    void PreDeviceCreated(VkDeviceCreateInfo& DeviceInfo) override final
    {
        AddToPNext(DeviceInfo, Synchronisation2Feature);
    }

private:
    VkPhysicalDeviceSynchronization2Features Synchronisation2Feature;
};

#define ADD_SIMPLE_EXTENSION(Array, ExtensionType, ExtensionName, Required) \
    Array.AddUnique(std::make_unique<ExtensionType>(ExtensionName, Required))
#define ADD_COMPLEX_ENTENSION(Array, ExtensionType) Array.AddUnique(std::make_unique<ExtensionType>())

FVulkanInstanceExtensionArray FVulkanPlatform::GetInstanceExtensions() const
{

    FVulkanInstanceExtensionArray InstanceExtension;
    RWindow::EnsureGLFWInit();

    uint32 glfwExtensionCount = 0;
    const char** glfwExtentsions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    for (uint32 i = 0; i < glfwExtensionCount; i++)
    {
        ADD_SIMPLE_EXTENSION(InstanceExtension, IInstanceVulkanExtension, glfwExtentsions[i], true);
    }

    ADD_SIMPLE_EXTENSION(InstanceExtension, IInstanceVulkanExtension, VK_KHR_SURFACE_EXTENSION_NAME, true);
#if VULKAN_DEBUGGING_ENABLED
    ADD_SIMPLE_EXTENSION(InstanceExtension, IInstanceVulkanExtension, VK_EXT_DEBUG_UTILS_EXTENSION_NAME, true);
#endif    // VULKAN_DEBUGGING_ENABLED
    return InstanceExtension;
}

FVulkanDeviceExtensionArray FVulkanPlatform::GetDeviceExtensions() const
{
    FVulkanDeviceExtensionArray DeviceExtension;

    ADD_SIMPLE_EXTENSION(DeviceExtension, IDeviceVulkanExtension, VK_KHR_SWAPCHAIN_EXTENSION_NAME, true);
    ADD_SIMPLE_EXTENSION(DeviceExtension, IDeviceVulkanExtension, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME, true);

    ADD_COMPLEX_ENTENSION(DeviceExtension, FDynamicRenderingExtension);
    ADD_COMPLEX_ENTENSION(DeviceExtension, FMaintenance5Extension);
    ADD_COMPLEX_ENTENSION(DeviceExtension, FSynchronisation2Extension);

    return DeviceExtension;
}

#undef ADD_SIMPLE_EXTENSION
#undef ADD_COMPLEX_ENTENSION

}    // namespace VulkanRHI
