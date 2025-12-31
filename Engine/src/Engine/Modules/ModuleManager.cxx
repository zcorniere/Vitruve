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
#if defined(PLATFORM_LINUX)
    // Try to see if it in the RPATH first
    TryLoadModule(ModuleName, Holder);
    if (Holder.State != EModuleState::Loaded)
#endif    // PLATFORM_LINUX
    {
        Holder.State = EModuleState::Error;
        Holder.ErrorStatus = EModuleErrorStatus::NotFound;
        Holder = FModuleHolder();
        for (const std::filesystem::path& Path: DLLSearchPaths)
        {
            LoadModuleWithPath(ModuleName, Path, Holder);
            if (Holder.State == EModuleState::Loaded)
            {
                break;
            }
        }
    }
    if (Holder.State == EModuleState::Unloaded)
    {
        LOG(LogModuleManager, Warning, "Module {:s} was not found", ModuleName);
    }
    else if (Holder.State == EModuleState::Error)
    {
        switch (Holder.ErrorStatus)
        {
            case EModuleErrorStatus::Malformed:
                LOG(LogModuleManager, Warning, "Module {:s} is malformed", ModuleName);
                break;
            case EModuleErrorStatus::EntryPointNotFound:
                LOG(LogModuleManager, Warning, "Module {:s} does not have the required entry point", ModuleName);
                break;
            case EModuleErrorStatus::NotFound:
                LOG(LogModuleManager, Warning, "Module {:s} not found in any of the search paths", ModuleName);
                break;
            case EModuleErrorStatus::None:
                checkNoEntry();
                break;
        }
        return nullptr;
    }
    else
    {
        Holder.Module->StartupModule();
        Holder.State = EModuleState::Started;
    }
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

static std::string GetModuleFullName(const std::string_view& ModuleName)
{
#if defined(PLATFORM_WINDOWS)
    return std::format("VitruveEngine_{:s}.dll", ModuleName);
#elif defined(PLATFORM_LINUX)
    return std::format("libVitruveEngine_{:s}.so", ModuleName);
#endif
}

void FModuleManager::TryLoadModule(const std::string_view& ModuleName, FModuleHolder& OutHolder)
{
    VIT_PROFILE_FUNC()

    const std::string FullName = GetModuleFullName(ModuleName);

    // Load the library object
    OutHolder.LibraryHolder = FPlatformMisc::LoadExternalModule(FullName);
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

void FModuleManager::LoadModuleWithPath(const std::string_view& ModuleName, const std::filesystem::path& Path,
                                        FModuleHolder& OutHolder)
{
    VIT_PROFILE_FUNC()

    const std::string FullName = GetModuleFullName(ModuleName);

    for (const std::filesystem::directory_entry& Entry: std::filesystem::recursive_directory_iterator(Path))
    {

        if (Entry.path().filename() != FullName)
        {
            continue;
        }
        return TryLoadModule(Entry.path().string(), OutHolder);
    }
}
