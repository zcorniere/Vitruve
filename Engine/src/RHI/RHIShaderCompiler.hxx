#pragma once

#include "Resources/RHIShader.hxx"

#include <slang-com-ptr.h>
#include <slang.h>

namespace ShaderResource
{

struct FPushConstantRange
{
    uint32 Offset = 0;
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

struct FReflectionData
{
    ERHIShaderType Type;
    std::string EntryPoint;

    TArray<ShaderResource::FStageIO> StageInput;
    TArray<ShaderResource::FStageIO> StageOutput;

    std::optional<ShaderResource::FPushConstantRange> PushConstants;

    TMap<uint32, TMap<uint32, ShaderResource::FDescriptorSetInfo>> DescriptorSetDeclaration;

    bool operator==(const FReflectionData& Other) const = default;

    static void Serialize(Serialization::FStreamWriter* Writer, const FReflectionData& Value);
    static void Deserialize(Serialization::FStreamReader* Reader, FReflectionData& OutValue);
};

}    // namespace ShaderResource

class ENGINE_API RRHIShaderCompiler : public RObject
{
    RTTI_DECLARE_TYPEINFO(RRHIShaderCompiler, RObject);

public:
    /// Which optimization level to use when compiling
    enum class EOptimizationLevel
    {
        /// No optimization, enable debug symbols
        None = 0,
        /// Optimize for size, no debug symbols
        Size,
        /// Optimize for performance, with debug symbols
        PerfWithDebug,
        /// Optimize for Performance, no debug symbols
        Performance,
    };

public:
    RRHIShaderCompiler();
    ~RRHIShaderCompiler();

    virtual Ref<RRHIShader> CreateShaderObject(const TArray<uint32>& InSPRIVCode,
                                               const ShaderResource::FReflectionData& InReflectionData) = 0;

    /// @brief Set the optimization level expected when compiling
    ///
    /// Changing the value does not trigger a recompilation
    void SetOptimizationLevel(EOptimizationLevel NewLevel);

    /// @brief Return a shader handle
    /// @param Path The path (internally used as ID) of the shader
    /// @param bForceCompile Should the shader be recompiled regardless of its cached status ?
    Ref<RRHIShader> Get(std::filesystem::path Path, const std::string& entryPoint, bool bForceCompile = false);

private:
    bool CreateSession();
    ShaderResource::FReflectionData GetReflection(const std::string_view& Path, slang::ProgramLayout* programLayout,
                                                  unsigned EntryPointIndex);

private:
    Slang::ComPtr<slang::ISession> session;

    TMap<std::string, std::string> PreprocessorMacros;
    TArray<std::string> WorkingDirectories;
    EOptimizationLevel Level = EOptimizationLevel::None;
};

DEFINE_PRINTABLE_TYPE(ShaderResource::FPushConstantRange, "PushConstantRange {{ Offset: {0}, Parameter: {1:#} }}",
                      Value.Offset, Value.Parameter)

DEFINE_PRINTABLE_TYPE(ShaderResource::FStageIO,
                      "StageIO {{ Name: \"{0}\", Type: {1}, Binding: {2}, Location: {3}, Offset: {4} }}", Value.Name,
                      magic_enum::enum_name(Value.Type), Value.Binding, Value.Location, Value.Offset)

DEFINE_PRINTABLE_TYPE(ShaderResource::FDescriptorSetInfo, " DescriptorSetInfo {{ Type: {0}, Parameter: {1:#} }}",
                      magic_enum::enum_name(Value.Type), Value.Parameter)

DEFINE_PRINTABLE_TYPE(
    ShaderResource::FReflectionData,
    "ReflectionData {{ StageInput: {0},\nStageOutput: {1},\nPushConstants: {2},\nDescriptor Sets: {3} }}",
    Value.StageInput, Value.StageOutput,
    Value.PushConstants.has_value() ? Value.PushConstants.value() : ShaderResource::FPushConstantRange{},
    Value.DescriptorSetDeclaration)
