#include "RHI/RHIShaderCompiler.hxx"

#include <slang.h>

ERHIShaderType FromSlang(SlangStage Stage)
{
    switch (Stage)
    {
        case SlangStage::SLANG_STAGE_COMPUTE:
            return ERHIShaderType::Compute;
        case SlangStage::SLANG_STAGE_FRAGMENT:
            return ERHIShaderType::Fragment;
        case SlangStage::SLANG_STAGE_VERTEX:
            return ERHIShaderType::Vertex;
        default:
            checkNoEntry();
    }
}

ShaderResource::FReflectionData RRHIShaderCompiler::GetReflection(slang::ProgramLayout* programLayout)
{
    ShaderResource::FReflectionData Data;
    Data.Type = FromSlang(programLayout->getEntryPointByIndex(0)->getStage());

    return Data;
}
