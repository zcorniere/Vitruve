#include "RHI/RHIShaderCompiler.hxx"

#include "Engine/Misc/DataLocation.hxx"
#include "Engine/Misc/Utils.hxx"

#include <slang-com-helper.h>
#include <slang-com-ptr.h>
#include <slang.h>

DECLARE_LOGGER_CATEGORY(Core, LogShaderCompiler, Info)

static Slang::ComPtr<slang::IGlobalSession> globalSession;

RRHIShaderCompiler::RRHIShaderCompiler()
{
    if (!globalSession)
    {
        SlangGlobalSessionDesc Desc;
        slang::createGlobalSession(&Desc, globalSession.writeRef());
    }

    WorkingDirectories.Add(DataLocationFinder::GetShaderPath());
}

RRHIShaderCompiler::~RRHIShaderCompiler()
{
}

void RRHIShaderCompiler::SetOptimizationLevel(EOptimizationLevel NewLevel)
{
    Level = NewLevel;
}

static TArray<slang::PreprocessorMacroDesc> GetPreprocessor(const TMap<std::string, std::string>& PreprocessorMacros)
{
    TArray<slang::PreprocessorMacroDesc> SearchPathChar;
    SearchPathChar.Reserve(PreprocessorMacros.Size());
    for (const auto& [Key, Value]: PreprocessorMacros)
    {
        slang::PreprocessorMacroDesc Desc{
            .name = Key.c_str(),
            .value = Value.c_str(),
        };
        SearchPathChar.Add(Desc);
    }
    return SearchPathChar;
}

static TArray<slang::CompilerOptionEntry> GetOptimisationLevel(RRHIShaderCompiler::EOptimizationLevel Level)
{
    using EOptimizationLevel = RRHIShaderCompiler::EOptimizationLevel;
    TArray<slang::CompilerOptionEntry> Entries;
    SlangOptimizationLevel SlangLevel;
    SlangDebugInfoLevel DebugLevel;

    switch (Level)
    {
        case EOptimizationLevel::None:
            SlangLevel = SLANG_OPTIMIZATION_LEVEL_NONE;
            DebugLevel = SLANG_DEBUG_INFO_LEVEL_STANDARD;
            break;
        case EOptimizationLevel::Size:
            SlangLevel = SLANG_OPTIMIZATION_LEVEL_DEFAULT;
            DebugLevel = SLANG_DEBUG_INFO_LEVEL_NONE;
            break;
        case EOptimizationLevel::PerfWithDebug:
            SlangLevel = SLANG_OPTIMIZATION_LEVEL_HIGH;
            DebugLevel = SLANG_DEBUG_INFO_LEVEL_MAXIMAL;
            break;
        case EOptimizationLevel::Performance:
            SlangLevel = SLANG_OPTIMIZATION_LEVEL_MAXIMAL;
            DebugLevel = SLANG_DEBUG_INFO_LEVEL_NONE;
            break;
    }

    slang::CompilerOptionEntry Entry;
    Entry.name = slang::CompilerOptionName::Optimization;
    Entry.value = slang::CompilerOptionValue{
        .kind = slang::CompilerOptionValueKind::Int,
        .intValue0 = static_cast<int32>(SlangLevel),
    };
    Entries.Add(Entry);

    Entry.name = slang::CompilerOptionName::DebugInformation;
    Entry.value = slang::CompilerOptionValue{
        .kind = slang::CompilerOptionValueKind::Int,
        .intValue0 = static_cast<int32>(DebugLevel),
    };
    Entries.Add(Entry);
    return Entries;
}

#define SLANG_DIAGNOSE(...)                                                                      \
    {                                                                                            \
        Slang::ComPtr<slang::IBlob> diagnostics;                                                 \
        __VA_ARGS__;                                                                             \
        if (diagnostics)                                                                         \
        {                                                                                        \
            LOG(LogShaderCompiler, Error, "{:s}", (const char*)diagnostics->getBufferPointer()); \
        }                                                                                        \
    }

bool RRHIShaderCompiler::CreateSession()
{
    if (session)
    {
        return true;
    }

    slang::SessionDesc sessionDesc;
    slang::TargetDesc targetDesc{
        .format = SLANG_SPIRV,
        .profile = globalSession->findProfile("spirv_1_3"),
    };
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;

    TArray<const char*> SearchPath;
    SearchPath.Reserve(WorkingDirectories.Size());
    for (const std::string& Path: WorkingDirectories)
    {
        SearchPath.Add(Path.c_str());
    }
    sessionDesc.searchPaths = SearchPath.Raw();
    sessionDesc.searchPathCount = SearchPath.Size();

    TArray<slang::PreprocessorMacroDesc> Preprocessor = GetPreprocessor(PreprocessorMacros);
    if (!Preprocessor.IsEmpty())
    {
        sessionDesc.preprocessorMacros = Preprocessor.Raw();
        sessionDesc.preprocessorMacroCount = Preprocessor.Size();
    }

    TArray<slang::CompilerOptionEntry> CompilerOption;
    CompilerOption.Add({.name = slang::CompilerOptionName::VulkanUseEntryPointName,
                        .value{
                            .intValue0 = 1,
                        }});
    CompilerOption.Append(GetOptimisationLevel(Level));
    sessionDesc.compilerOptionEntryCount = CompilerOption.Size();
    sessionDesc.compilerOptionEntries = CompilerOption.Raw();

    return SLANG_OK == globalSession->createSession(sessionDesc, session.writeRef());
}

Ref<RRHIShader> RRHIShaderCompiler::Get(std::filesystem::path Path, const std::string& entryPoint, bool bForceCompile)
{
    bool bSessionOK = CreateSession();
    check(bSessionOK);

    slang::IModule* module = nullptr;
    SLANG_DIAGNOSE(module = session->loadModule(Path.string().c_str(), diagnostics.writeRef()));
    if (!module)
    {
        return nullptr;
    }

    Slang::ComPtr<slang::IEntryPoint> EntryPoint;
    module->findEntryPointByName(entryPoint.c_str(), EntryPoint.writeRef());

    TArray<slang::IComponentType*> componentTypes;
    componentTypes.Add(module);
    componentTypes.Add(EntryPoint);

    Slang::ComPtr<slang::IComponentType> LinkedProgram;
    SlangResult Result;
    SLANG_DIAGNOSE(Result = session->createCompositeComponentType(componentTypes.Raw(), componentTypes.Size(),
                                                                  LinkedProgram.writeRef(), diagnostics.writeRef()));
    if (Result == SLANG_FAIL)
    {
        return nullptr;
    }

    Slang::ComPtr<slang::IBlob> spirvCode;
    SLANG_DIAGNOSE(Result = LinkedProgram->getEntryPointCode(0, 0, spirvCode.writeRef(), diagnostics.writeRef()));
    if (Result == SLANG_FAIL)
    {
        return nullptr;
    }
    slang::ProgramLayout* layout = LinkedProgram->getLayout(0);
    ShaderResource::FReflectionData Reflection = GetReflection(module->getFilePath(), layout, 0);
    Reflection.EntryPoint = entryPoint;

    TArray<uint32> SPIRVCode(reinterpret_cast<const uint32*>(spirvCode->getBufferPointer()),
                             spirvCode->getBufferSize() / sizeof(uint32));
    return CreateShaderObject(SPIRVCode, Reflection);
}
