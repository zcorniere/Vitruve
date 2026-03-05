#pragma once

#include "Resources/RHIShader.hxx"

#include <slang-com-ptr.h>
#include <slang.h>

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

struct FReflectionData
{
    ERHIShaderType Type;

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

    virtual Ref<RRHIShader> CreateShaderObject(ERHIShaderType Type, const TArray<uint32>& InSPRIVCode,
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
    ShaderResource::FReflectionData GetReflection(slang::ProgramLayout* programLayout);

private:
    Slang::ComPtr<slang::ISession> session;

    TMap<std::string, std::string> PreprocessorMacros;
    TArray<std::string> WorkingDirectories;
    EOptimizationLevel Level = EOptimizationLevel::None;
};
