#include "VulkanRHI/DescriptorPoolManager.hxx"

#include "VulkanRHI/Resources/VulkanShader.hxx"
#include "VulkanRHI/VulkanDevice.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogDescriptorSetManager, Warning)

namespace VulkanRHI
{

FDescriptorSetManager::FDescriptorSetManager(FVulkanDevice* InDevice, Ref<RVulkanGraphicsPipeline>& GraphicsPipeline)
    : IDeviceChild(InDevice)
    , AssociatedPipeline(GraphicsPipeline)
{
    CollectDescriptorSetInfo();
}

FDescriptorSetManager::~FDescriptorSetManager()
{
    Destroy();
}

void FDescriptorSetManager::Destroy()
{
    if (DescriptorSets.IsEmpty() && DescriptorPoolHandle != VK_NULL_HANDLE)
    {
        RHI::DeferedDeletion(
            [Handle = DescriptorPoolHandle, Device = Device]
            { VulkanAPI::vkDestroyDescriptorPool(Device->GetHandle(), Handle, VULKAN_CPU_ALLOCATOR); });
    }
    DescriptorSets.Clear();

    if (DescriptorPoolHandle != VK_NULL_HANDLE)
    {
        RHI::DeferedDeletion(
            [Handle = DescriptorPoolHandle, Device = Device]
            { VulkanAPI::vkDestroyDescriptorPool(Device->GetHandle(), Handle, VULKAN_CPU_ALLOCATOR); });
    }
    DescriptorPoolHandle = VK_NULL_HANDLE;
}

void FDescriptorSetManager::Bake()
{
    CreateDescriptorPool(InputResources.Size());
    CreateDescriptorSets(InputResources.Size());

    TArray<VkWriteDescriptorSet> WriteDescriptorSetsArray;
    for (auto& [Set, Bindings]: WriteDescriptorSet)
    {
        for (auto& [Binding, WriteDescriptor]: Bindings)
        {
            FRenderPassInput& RenderPassInput = InputResources[Set][Binding];
            if (RenderPassInput.Input[0] == nullptr)
            {
                LOG(LogDescriptorSetManager, Warning, "No input set for binding {} in set {}", Binding, Set);
                continue;
            }

            WriteDescriptorSetsArray.Emplace(WriteDescriptor);
            WriteDescriptorSetsArray.Back().dstSet = DescriptorSets[Set];

            switch (RenderPassInput.Type)
            {
                case ERenderPassInputType::Texture:
                {
                    Ref<RVulkanTexture> Image = RenderPassInput.Input[0].As<RVulkanTexture>();
                    WriteDescriptorSetsArray.Back().pImageInfo = &Image->GetDescriptorImageInfo();
                }
                break;

                case ERenderPassInputType::StorageBuffer:
                {
                    Ref<RVulkanBuffer> Buffer = RenderPassInput.Input[0].As<RVulkanBuffer>();
                    WriteDescriptorSetsArray.Back().pBufferInfo = &Buffer->GetDescriptorBufferInfo();
                }
                break;

                default:
                {
                    LOG(LogDescriptorSetManager, Error, "Unsupported render pass input type: {}",
                        static_cast<int>(RenderPassInput.Type));
                    continue;
                }
            }
        }
    }

    if (!WriteDescriptorSetsArray.IsEmpty())
    {
        VulkanAPI::vkUpdateDescriptorSets(Device->GetHandle(), WriteDescriptorSetsArray.Size(),
                                          WriteDescriptorSetsArray.Raw(), 0, nullptr);
    }
}

void FDescriptorSetManager::Bind(VkCommandBuffer CmdBuffer, VkPipelineLayout PipelineLayout,
                                 VkPipelineBindPoint BindPoint)
{
    if (DescriptorSets.IsEmpty())
    {
        LOG(LogDescriptorSetManager, Error, "Descriptor sets are empty. Cannot bind.");
        return;
    }
    VulkanAPI::vkCmdBindDescriptorSets(CmdBuffer, BindPoint, PipelineLayout, 0, DescriptorSets.Size(),
                                       DescriptorSets.Raw(), 0, nullptr);
}

void FDescriptorSetManager::InvalidateAndUpdate()
{
    TMap<uint32, TMap<uint32, FRenderPassInput>> InvalidatedInput;

    for (auto& [Set, Inputs]: InputResources)
    {
        for (auto& [Binding, Input]: Inputs)
        {
            switch (Input.Type)
            {
                case ERenderPassInputType::StorageBuffer:
                {
                    const RVulkanBuffer* const Buffer = Input.Input[0].AsRaw<RVulkanBuffer>();
                    if (!Buffer)
                    {
                        continue;
                    }

                    const VkDescriptorBufferInfo& Info = Buffer->GetDescriptorBufferInfo();
                    const VkWriteDescriptorSet& SetWrite = WriteDescriptorSet[Set][Binding];
                    if (!SetWrite.pBufferInfo || Info.buffer != SetWrite.pBufferInfo->buffer)
                    {
                        InvalidatedInput.FindOrAdd(Set).FindOrAdd(Binding) = Input;
                    }
                }
                break;
                case ERenderPassInputType::Texture:
                {
                    const RVulkanTexture* const Texture = Input.Input[0].AsRaw<RVulkanTexture>();
                    if (!Texture)
                    {
                        continue;
                    }

                    const VkDescriptorImageInfo& Info = Texture->GetDescriptorImageInfo();
                    const VkWriteDescriptorSet& SetWrite = WriteDescriptorSet[Set][Binding];
                    if (!SetWrite.pImageInfo || Info.imageView != SetWrite.pImageInfo->imageView)
                    {
                        InvalidatedInput.FindOrAdd(Set).FindOrAdd(Binding) = Input;
                    }
                }
                default:
                    break;
            }
        }
    }

    if (InvalidatedInput.IsEmpty())
        return;

    for (auto& [Set, Inputs]: InvalidatedInput)
    {

        TArray<VkWriteDescriptorSet> WriteDescriptorSetsToUpdate;
        for (auto& [Binding, Input]: Inputs)
        {
            VkWriteDescriptorSet& WriteDescriptor = WriteDescriptorSet[Set][Binding];
            WriteDescriptor.dstSet = DescriptorSets[Set];
            switch (Input.Type)
            {
                case ERenderPassInputType::StorageBuffer:
                {
                    const VkDescriptorBufferInfo& Info = Input.Input[0].As<RVulkanBuffer>()->GetDescriptorBufferInfo();
                    WriteDescriptor.pBufferInfo = &Info;
                }
                break;
                case ERenderPassInputType::Texture:
                {
                    const VkDescriptorImageInfo& Info = Input.Input[0].As<RVulkanTexture>()->GetDescriptorImageInfo();
                    WriteDescriptor.pImageInfo = &Info;
                }
                break;
                default:
                    break;
            }
            WriteDescriptorSetsToUpdate.Emplace(WriteDescriptor);
        }

        VulkanAPI::vkUpdateDescriptorSets(Device->GetHandle(), WriteDescriptorSetsToUpdate.Size(),
                                          WriteDescriptorSetsToUpdate.Raw(), 0, nullptr);
    }
}

void FDescriptorSetManager::SetInput(std::string_view Name, const Ref<RVulkanBuffer>& Buffer)
{
    const FRenderPassInputDeclaration* const Declaration = GetInputDeclaration(Name);
    if (Declaration)
    {
        InputResources[Declaration->Set][Declaration->Binding].Set(Buffer);
    }
    else
    {
        LOG(LogDescriptorSetManager, Warning, "Input declaration not found for {}", Name);
    }
}

void FDescriptorSetManager::SetInput(std::string_view Name, const Ref<RVulkanTexture>& Texture)
{
    const FRenderPassInputDeclaration* const Declaration = GetInputDeclaration(Name);
    if (Declaration)
    {
        InputResources[Declaration->Set][Declaration->Binding].Set(Texture);
    }
    else
    {
        LOG(LogDescriptorSetManager, Warning, "Input declaration not found for {}", Name);
    }
}

const FDescriptorSetManager::FRenderPassInputDeclaration*
FDescriptorSetManager::GetInputDeclaration(std::string_view name) const
{
    std::string nameStr(name);
    return InputDeclaration.Find(nameStr);
}

void FDescriptorSetManager::CreateDescriptorPool(unsigned InMaxSets)
{
    VkDescriptorPoolCreateInfo CreateInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = InMaxSets,
        .poolSizeCount = DescriptorPoolSizes.Size(),
        .pPoolSizes = DescriptorPoolSizes.Raw(),
    };
    ensure(DescriptorPoolHandle == VK_NULL_HANDLE);
    VK_CHECK_RESULT(VulkanAPI::vkCreateDescriptorPool(Device->GetHandle(), &CreateInfo, VULKAN_CPU_ALLOCATOR,
                                                      &DescriptorPoolHandle));
}

void FDescriptorSetManager::CreateDescriptorSets(unsigned InMaxSets)
{
    VkDescriptorSetAllocateInfo AllocateInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = DescriptorPoolHandle,
        .descriptorSetCount = InMaxSets,
        .pSetLayouts = AssociatedPipeline->GetDescriptorSetLayouts().Raw(),
    };
    ensure(DescriptorSets.IsEmpty());

    DescriptorSets.Resize(InMaxSets);
    VK_CHECK_RESULT(VulkanAPI::vkAllocateDescriptorSets(Device->GetHandle(), &AllocateInfo, DescriptorSets.Raw()));
}

void FDescriptorSetManager::CollectDescriptorSetInfo()
{

    auto DescriptorTypeToRenderPassInputType = [](ShaderResource::FDescriptorSetInfo::EDescriptorType Type)
    {
        switch (Type)
        {
            case ShaderResource::FDescriptorSetInfo::EDescriptorType::StorageBuffer:
                return ERenderPassInputType::StorageBuffer;
            case ShaderResource::FDescriptorSetInfo::EDescriptorType::UniformBuffer:
                return ERenderPassInputType::UniformBuffer;
            case ShaderResource::FDescriptorSetInfo::EDescriptorType::Sampler:
                return ERenderPassInputType::Texture;
        }
        checkNoEntry();
    };

    for (const WeakRef<RVulkanShader>& Shader: AssociatedPipeline->GetShaders())
    {
        for (auto& [Set, Layout]: Shader->GetReflectionData().DescriptorSetDeclaration)
        {
            for (auto& [Binding, Parameter]: Layout)
            {
                VkDescriptorPoolSize* Found = DescriptorPoolSizes.FindByLambda(
                    [Parameter](const VkDescriptorPoolSize& Iter)
                    { return Iter.type == DescriptorTypeToVkDescriptorType(Parameter.Type); });
                if (!Found)
                {
                    Found = &DescriptorPoolSizes.Emplace(VkDescriptorPoolSize{
                        .type = DescriptorTypeToVkDescriptorType(Parameter.Type),
                        .descriptorCount = 0,
                    });
                }
                Found->descriptorCount += 1;

                InputDeclaration.Insert(Parameter.Parameter.Name,
                                        FRenderPassInputDeclaration{
                                            .Type = DescriptorTypeToRenderPassInputType(Parameter.Type),
                                            .Set = Set,
                                            .Binding = Binding,
                                            .Count = 1,
                                            .Name = Parameter.Parameter.Name,
                                        });

                FRenderPassInput& InputResource = InputResources.FindOrAdd(Set).FindOrAdd(Binding);
                InputResource.Type = DescriptorTypeToRenderPassInputType(Parameter.Type);
                InputResource.Input.Resize(1);

                WriteDescriptorSet.FindOrAdd(Set).FindOrAdd(Binding) = {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .pNext = nullptr,
                    .dstSet = VK_NULL_HANDLE,    // Will be set later
                    .dstBinding = Binding,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = DescriptorTypeToVkDescriptorType(Parameter.Type),
                    .pImageInfo = nullptr,
                    .pBufferInfo = nullptr,
                    .pTexelBufferView = nullptr,
                };
            };
        }
    }
}

}    // namespace VulkanRHI
