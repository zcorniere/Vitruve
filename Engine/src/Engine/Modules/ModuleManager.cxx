#include "Engine/Modules/ModuleManager.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogModuleManager, Info)

FModuleManager& FModuleManager::Get()
{
    static FModuleManager Instance;
    return Instance;
}

FModuleManager::FModuleManager()
{
    // By default, we search in the current directory
    DLLSearchPaths.Add(FPlatform::GetExecutablePath().parent_path());
}

void FModuleManager::AddDLLSearchPath(const std::filesystem::path& Path)
{
    DLLSearchPaths.Add(Path);
}

IModuleInterface* FModuleManager::LoadModule(const std::string_view& ModuleName)
{
    VIT_PROFILE_FUNC()

    FModuleHolder& Holder = Modules.FindOrAdd(std::string(ModuleName));
    if (Holder.State == EModuleState::Loaded)
    {
        return Holder.Module;
    }
    if (Holder.State == EModuleState::Error)
    {
        LOG(LogModuleManager, Warning, "Module {:s} is in error state, cannot load it. Reason: {:s}", ModuleName,
            magic_enum::enum_name(Holder.ErrorStatus));
        return nullptr;
    }

    LOG(LogModuleManager, Info, "Loading module {:s}", ModuleName);
    Holder.State = EModuleState::Error;
    Holder.ErrorStatus = EModuleErrorStatus::NotFound;
    for (const std::filesystem::path& Path: DLLSearchPaths)
    {
        LoadModuleWithPath(ModuleName, Path, Holder);
        if (Holder.State == EModuleState::Loaded)
        {
            break;
        }
    }
    if (Holder.State == EModuleState::Error)
    {
        if (Holder.ErrorStatus == EModuleErrorStatus::EntryPointNotFound)
        {
            LOG(LogModuleManager, Warning, "Module {:s} does not have the required entry point", ModuleName);
            Holder.State = EModuleState::Error;
            return nullptr;
        }
        if (Holder.ErrorStatus == EModuleErrorStatus::NotFound)
        {
            LOG(LogModuleManager, Warning, "Module {:s} not found in any of the search paths", ModuleName);
            Holder.State = EModuleState::Error;
            return nullptr;
        }
    }
    Holder.Module->StartupModule();
    Holder.State = EModuleState::Started;
    return Holder.Module;
}

void FModuleManager::UnloadModule(const std::string_view& ModuleName)
{
    VIT_PROFILE_FUNC()

    LOG(LogModuleManager, Info, "Unloading module {:s}", ModuleName);
    TPair<std::string, FModuleHolder> Pair;
    if (!Modules.Remove(std::string(ModuleName), &Pair))
    {
        LOG(LogModuleManager, Warning, "Module {:s} is not loaded", ModuleName);
        return;
    }

    if (Pair.Get<1>().State != EModuleState::Started)
    {
        LOG(LogModuleManager, Warning, "Module {:s} is not started, cannot unload it", ModuleName);
        return;
    }
    Pair.Get<1>().Module->ShutdownModule();

    // Remove our module object
    delete Pair.Get<1>().Module;

    // Unload the library code
    delete Pair.Get<1>().LibraryHolder;
}

void FModuleManager::LoadModuleWithPath(const std::string_view& ModuleName, const std::filesystem::path& Path,
                                        FModuleHolder& OutHolder)
{
    VIT_PROFILE_FUNC()

    const std::string FullPath = std::format("libVitruveEngine_{:s}.{:s}", ModuleName,
#if defined(PLATFORM_WINDOWS)
                                             "dll"
#elif defined(PLATFORM_LINUX)
                                             "so"
#endif
    );

    for (const std::filesystem::directory_entry& Entry: std::filesystem::recursive_directory_iterator(Path))
    {

        if (Entry.path().filename() != FullPath)
        {
            continue;
        }

        // Load the library object
        OutHolder.LibraryHolder = FPlatformMisc::LoadExternalModule(Entry.path().string());
        if (!OutHolder.LibraryHolder->IsValid())
        {
            OutHolder.State = EModuleState::Error;
            OutHolder.ErrorStatus = EModuleErrorStatus::Malformed;
            delete OutHolder.LibraryHolder;
            return;
        }

        // Grab our entry point
        using ModuleCreateFn = IModuleInterface* (*)();
        ModuleCreateFn CreateModule = OutHolder.LibraryHolder->GetSymbol<ModuleCreateFn>("CreateModule");
        if (!CreateModule)
        {
            OutHolder.State = EModuleState::Error;
            OutHolder.ErrorStatus = EModuleErrorStatus::EntryPointNotFound;
            delete OutHolder.LibraryHolder;
            return;
        }

        // Create our object
        OutHolder.Module = CreateModule();
        OutHolder.State = EModuleState::Loaded;
        OutHolder.ErrorStatus = EModuleErrorStatus::None;
        return;
    }
}
