#pragma once

#include "Engine/RHI/RHIResource.hxx"
#include "VulkanRHI/Resources/VulkanBuffer.hxx"
#include "VulkanRHI/Resources/VulkanTexture.hxx"

namespace VulkanRHI
{
class RVulkanShader;
class RVulkanGraphicsPipeline;

class FDescriptorSetManager : public IDeviceChild
{
public:
    enum class ERenderPassInputType
    {
        None = 0,
        Texture,
        StorageBuffer,
    };

    struct FRenderPassInputDeclaration
    {
        ERenderPassInputType Type = ERenderPassInputType::None;
        uint32_t Set = 0;
        uint32_t Binding = 0;
        uint32_t Count = 0;
        std::string Name;
    };

    struct FRenderPassInput
    {
        FRenderPassInput() = default;
        FRenderPassInput(Ref<RVulkanBuffer> StorageBuffer): Type(ERenderPassInputType::StorageBuffer)
        {
            Input.Add(StorageBuffer);
        }
        FRenderPassInput(Ref<RVulkanTexture> Texture): Type(ERenderPassInputType::Texture)
        {
            Input.Add(Texture);
        }

        void Set(Ref<RVulkanBuffer> StorageBuffer, uint32_t index = 0)
        {
            Type = ERenderPassInputType::StorageBuffer;
            Input[index] = StorageBuffer;
        }
        void Set(Ref<RVulkanTexture> StorageBuffer, uint32_t index = 0)
        {
            Type = ERenderPassInputType::Texture;
            Input[index] = StorageBuffer;
        }

        ERenderPassInputType Type = ERenderPassInputType::None;
        TArray<Ref<RRHIResource>> Input = {};
    };

public:
    FDescriptorSetManager(FVulkanDevice* InDevice, Ref<RVulkanGraphicsPipeline>& GraphicsPipeline);
    virtual ~FDescriptorSetManager();

    void Destroy();

    void Bake();
    void Bind(VkCommandBuffer Cmd, VkPipelineLayout PipelineLayout, VkPipelineBindPoint BindPoint);
    void InvalidateAndUpdate();

    void SetInput(std::string_view Name, const Ref<RVulkanBuffer>& Buffer);
    void SetInput(std::string_view Name, const Ref<RVulkanTexture>& Texture);

    VkDescriptorPool GetHandle() const
    {
        return DescriptorPoolHandle;
    }
    VkDescriptorSet GetDescriptorSet(unsigned Set) const;
    TArray<VkDescriptorSet> GetDescriptorSets() const
    {
        return DescriptorSets;
    }

    const FRenderPassInputDeclaration* GetInputDeclaration(std::string_view name) const;

private:
    void CreateDescriptorPool(unsigned InMaxSets);
    void CreateDescriptorSets();
    void CollectDescriptorSetInfo();

private:
    VkDescriptorPool DescriptorPoolHandle = VK_NULL_HANDLE;
    Ref<RVulkanGraphicsPipeline> AssociatedPipeline = nullptr;

    TArray<VkDescriptorPoolSize> DescriptorPoolSizes = {};
    TArray<VkDescriptorSet> DescriptorSets = {};

    TMap<std::string, FRenderPassInputDeclaration> InputDeclaration = {};
    TMap<uint32, TMap<uint32, FRenderPassInput>> InputResources = {};
    TMap<uint32, TMap<uint32, VkWriteDescriptorSet>> WriteDescriptorSet = {};
};

}    // namespace VulkanRHI
