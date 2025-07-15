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
    CreateDescriptorPool(1);
    CreateDescriptorSets();

    TArray<VkWriteDescriptorSet> WriteDescriptorSetsArray;
    for (auto& [Set, Bindings]: WriteDescriptorSet)
    {
        for (auto& [Binding, WriteDescriptor]: Bindings)
        {
            WriteDescriptorSetsArray.Emplace(WriteDescriptor);
            WriteDescriptorSetsArray.Back().dstSet = DescriptorSets[Set];

            Ref<RVulkanBuffer> Buffer = InputResources[Set][Binding].Input[0].As<RVulkanBuffer>();
            WriteDescriptorSetsArray.Back().pBufferInfo = &Buffer->GetDescriptorBufferInfo();
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
                    if (SetWrite.pBufferInfo && Info.buffer != SetWrite.pBufferInfo->buffer)
                    {
                        InvalidatedInput[Set][Binding] = Input;
                    }
                }
                break;
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
            switch (Input.Type)
            {
                case ERenderPassInputType::StorageBuffer:
                {
                    const VkDescriptorBufferInfo& Info = Input.Input[0].As<RVulkanBuffer>()->GetDescriptorBufferInfo();
                    WriteDescriptor.pBufferInfo = &Info;
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

void FDescriptorSetManager::CreateDescriptorSets()
{
    VkDescriptorSetAllocateInfo AllocateInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = DescriptorPoolHandle,
        .descriptorSetCount = 1,
        .pSetLayouts = AssociatedPipeline->GetDescriptorSetLayouts().Raw(),
    };
    ensure(DescriptorSets.IsEmpty());

    DescriptorSets.Resize(1);
    VK_CHECK_RESULT(VulkanAPI::vkAllocateDescriptorSets(Device->GetHandle(), &AllocateInfo, DescriptorSets.Raw()));
}

void FDescriptorSetManager::CollectDescriptorSetInfo()
{

    auto DescriptorTypeToRenderPassInputType = [](ShaderResource::FDescriptorSetInfo::EDescriptorType Type)
    {
        switch (Type)
        {
            case ShaderResource::FDescriptorSetInfo::EDescriptorType::StorageBuffer:
            case ShaderResource::FDescriptorSetInfo::EDescriptorType::UniformBuffer:
                return ERenderPassInputType::StorageBuffer;
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
                DescriptorPoolSizes
                    .FindByLambda<true>([Binding](const VkDescriptorPoolSize& Iter) { return Iter.type == Binding; })
                    ->descriptorCount += 1;

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
