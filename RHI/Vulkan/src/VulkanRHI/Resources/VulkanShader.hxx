#pragma once

#include "Engine/Serialization/StreamReader.hxx"
#include "Engine/Serialization/StreamWriter.hxx"

#include "Engine/RHI/Resources/RHIShader.hxx"

namespace VulkanRHI
{

class FVulkanDevice;

namespace ShaderResource
{

    struct FPushConstantRange
    {
        uint32 Offset = 0;
        uint32 Size = 0;

        ::RTTI::FParameter Parameter;

        bool operator==(const FPushConstantRange&) const = default;
    };

    struct FStageIO
    {
        std::string Name;
        EVertexElementType Type;
        uint32 Binding = 0;
        uint32 Location = 0;
        uint32 Offset = 0;

        bool operator==(const FStageIO&) const = default;
    };

    struct FDescriptorSetInfo
    {
        enum class EDescriptorType
        {
            StorageBuffer,
            UniformBuffer,
            Sampler,
        } Type;
        RTTI::FParameter Parameter;

        bool operator==(const FDescriptorSetInfo& Other) const = default;
    };

}    // namespace ShaderResource

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

    struct FReflectionData
    {
        TArray<ShaderResource::FStageIO> StageInput;
        TArray<ShaderResource::FStageIO> StageOutput;

        std::optional<ShaderResource::FPushConstantRange> PushConstants;

        TMap<uint32, TMap<uint32, ShaderResource::FDescriptorSetInfo>> DescriptorSetDeclaration;

        bool operator==(const FReflectionData& Other) const = default;

        static void Serialize(Serialization::FStreamWriter* Writer, const FReflectionData& Value);
        static void Deserialize(Serialization::FStreamReader* Reader, FReflectionData& OutValue);
    };

public:
    RVulkanShader() = delete;
    RVulkanShader(ERHIShaderType Type, const TArray<uint32>& InSPRIVCode, const FReflectionData& InReflectionData);
    virtual ~RVulkanShader();

    const FReflectionData& GetReflectionData() const
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
    const FReflectionData m_ReflectionData;

    ERHIShaderType Type;
    VkShaderModuleCreateInfo ShaderModuleCreateInfo;

    friend class VulkanShaderCompiler;
};

}    // namespace VulkanRHI

DEFINE_PRINTABLE_TYPE(VulkanRHI::ShaderResource::FPushConstantRange,
                      "PushConstantRange {{ Offset: {0}, Size: {1}, Parameter: {2:#} }}", Value.Offset, Value.Size,
                      Value.Parameter)

DEFINE_PRINTABLE_TYPE(VulkanRHI::ShaderResource::FStageIO,
                      "StageIO {{ Name: \"{0}\", Type: {1}, Binding: {2}, Location: {3}, Offset: {4} }}", Value.Name,
                      magic_enum::enum_name(Value.Type), Value.Binding, Value.Location, Value.Offset)

DEFINE_PRINTABLE_TYPE(VulkanRHI::ShaderResource::FDescriptorSetInfo,
                      " DescriptorSetInfo {{ Type: {0}, Parameter: {1:#} }}", magic_enum::enum_name(Value.Type),
                      Value.Parameter)

DEFINE_PRINTABLE_TYPE(
    VulkanRHI::RVulkanShader::FReflectionData,
    "ReflectionData {{ StageInput: {0},\nStageOutput: {1},\nPushConstants: {2},\nDescriptor Sets: {3} }}",
    Value.StageInput, Value.StageOutput,
    Value.PushConstants.has_value() ? Value.PushConstants.value() : VulkanRHI::ShaderResource::FPushConstantRange{},
    Value.DescriptorSetDeclaration)
