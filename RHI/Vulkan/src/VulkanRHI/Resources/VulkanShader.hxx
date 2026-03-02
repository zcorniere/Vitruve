#pragma once

#include "RHI/RHIShaderCompiler.hxx"
#include "RHI/Resources/RHIShader.hxx"

namespace VulkanRHI
{

class FVulkanDevice;

class RVulkanShader : public RRHIShader
{
    RTTI_DECLARE_TYPEINFO(RVulkanShader, RRHIShader);

public:
    class RVulkanShaderHandle : public RObject, public IDeviceChild
    {
        RTTI_DECLARE_TYPEINFO(RVulkanShaderHandle, RObject);

    public:
        RVulkanShaderHandle(FVulkanDevice* InDevice, const VkShaderModuleCreateInfo& Info);
        virtual ~RVulkanShaderHandle();

        virtual void SetName(std::string_view Name) override;

    public:
        VkShaderModule Handle;
    };

public:
    RVulkanShader() = delete;
    RVulkanShader(ERHIShaderType Type, const TArray<uint32>& InSPRIVCode,
                  const ShaderResource::FReflectionData& InReflectionData);
    virtual ~RVulkanShader();

    const ShaderResource::FReflectionData& GetReflectionData() const
    {
        return m_ReflectionData;
    }

    const VkShaderModuleCreateInfo& GetShaderModuleCreateInfo() const;
    const char* GetEntryPoint() const;

    constexpr ERHIShaderType GetShaderType() const
    {
        return Type;
    }

private:
    const TArray<uint32> SPIRVCode;
    const ShaderResource::FReflectionData m_ReflectionData;

    ERHIShaderType Type;
    VkShaderModuleCreateInfo ShaderModuleCreateInfo;

    friend class VulkanShaderCompiler;
};

}    // namespace VulkanRHI
