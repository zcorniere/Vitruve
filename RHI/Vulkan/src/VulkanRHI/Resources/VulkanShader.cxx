#include "VulkanRHI/Resources/VulkanShader.hxx"

#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanUtils.hxx"

namespace VulkanRHI
{

// Serialization

RVulkanShader::RVulkanShaderHandle::RVulkanShaderHandle(FVulkanDevice* InDevice, const VkShaderModuleCreateInfo& Info)
    : IDeviceChild(InDevice)
{
    VK_CHECK_RESULT(VulkanAPI::vkCreateShaderModule(Device->GetHandle(), &Info, VULKAN_CPU_ALLOCATOR, &Handle));
}

RVulkanShader::RVulkanShaderHandle::~RVulkanShaderHandle()
{
    RHI::DeferedDeletion([Handle = this->Handle, Device = this->Device]
                         { VulkanAPI::vkDestroyShaderModule(Device->GetHandle(), Handle, VULKAN_CPU_ALLOCATOR); });
}

void RVulkanShader::RVulkanShaderHandle::SetName(std::string_view Name)
{
    Super::SetName(Name);
    VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_SHADER_MODULE, Handle, "{:s}", Name);
}

RVulkanShader::RVulkanShader(ERHIShaderType Type, const TArray<uint32>& InSPIRVCode,
                             const FReflectionData& InReflectionData)
    : Super(Type)
    , SPIRVCode(InSPIRVCode)
    , m_ReflectionData(InReflectionData)
    , Type(Type)
{
    ShaderModuleCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = SPIRVCode.Size() * sizeof(uint32),
        .pCode = SPIRVCode.Raw(),
    };
}

RVulkanShader::~RVulkanShader()
{
}

const VkShaderModuleCreateInfo& RVulkanShader::GetShaderModuleCreateInfo() const
{
    return ShaderModuleCreateInfo;
}

const char* RVulkanShader::GetEntryPoint() const
{
    return "main";
}

}    // namespace VulkanRHI
