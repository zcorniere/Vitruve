#include "VulkanRHI/VulkanUtils.hxx"

#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanMemoryManager.hxx"
#include "VulkanRHI/VulkanRHI.hxx"

#include "Engine/Misc/ConsoleVariable.hxx"

#include <magic_enum/magic_enum.hpp>

static TConsoleVariable<bool> CVar_Vulkan_DumpVmaOnOutOfMemory(
    "vulkan.DumpVmaOnOutOfMemory", false,
    "If true, dump the VMA memory state when an out of memory error occurs in Vulkan.");

namespace VulkanRHI
{

void VulkanCheckResult(VkResult Result, const char* VulkanFunction, const std::source_location& location)
{
    bool bDumpMemory = false;
    std::string_view ErrorString = magic_enum::enum_name(Result);

    switch (Result)
    {
        case VK_ERROR_OUT_OF_HOST_MEMORY:
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            bDumpMemory = true;
            break;
        default:
            break;
    }

    if (bDumpMemory && CVar_Vulkan_DumpVmaOnOutOfMemory.GetValue())
    {
        LOG(LogVulkanRHI, Fatal, "VMA DUMP : \n{}",
            GetVulkanDynamicRHI()->GetDevice()->GetMemoryManager()->GetVMADumpString());
    }

    LOG(LogVulkanRHI, Fatal, "{} failed, VkResult={:s}\n\tat {}:{} with error {}", VulkanFunction,
        string_VkResult(Result), location.file_name(), location.line(), ErrorString);
    check(false);
}

}    // namespace VulkanRHI
