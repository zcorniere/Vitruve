#pragma once

#include "Engine/Modules/ModuleInterface.hxx"

class ENGINE_API FModuleManager
{
public:
    enum class EModuleState
    {
        Unloaded,
        Loaded,
        Started,
        Error
    };

    enum class EModuleErrorStatus
    {
        None,
        NotFound,
        Malformed,
        EntryPointNotFound,
    };

private:
    struct FModuleHolder
    {
        EModuleState State = EModuleState::Unloaded;
        EModuleErrorStatus ErrorStatus = EModuleErrorStatus::None;
        IModuleInterface* Module = nullptr;
        IExternalModule* LibraryHolder = nullptr;
    };

public:
    static FModuleManager& Get();

private:
    FModuleManager();

public:
    void AddDLLSearchPath(const std::filesystem::path& Path);

    IModuleInterface* LoadModule(const std::string_view& ModuleName);
    void UnloadModule(const std::string_view& ModuleName);

private:
    void LoadModuleWithPath(const std::string_view& ModuleName, const std::filesystem::path& Path,
                            FModuleHolder& OutHolder);

private:
    TArray<std::filesystem::path> DLLSearchPaths;
    TMap<std::string, FModuleHolder> Modules;
};
