#pragma once

#include "Engine/Platforms/PlatformMisc.hxx"
// IWYU pragma: private, include "FPlatformMisc.hxx"

/// @brief Linux implementation of the IExternalModule interface
class ENGINE_API FLinuxExternalModule final : public IExternalModule
{
    RTTI_DECLARE_TYPEINFO(FLinuxExternalModule, IExternalModule);

public:
    /// @copydoc IExternalModule::IExternalModule
    explicit FLinuxExternalModule(std::string_view ModulePath);
    virtual ~FLinuxExternalModule();

    virtual bool IsValid() const override
    {
        return ModuleHandle != nullptr;
    }

private:
    virtual void* GetSymbol_Internal(std::string_view SymbolName) const override;

private:
    void* ModuleHandle = nullptr;
};

/// @brief Miscellaneous Linux feature
class ENGINE_API FLinuxMisc : public FGenericMisc
{
public:
    /// @copydoc GenericMisc::DisplayMessageBox
    static EBoxReturnType DisplayMessageBox(EBoxMessageType MsgType, const std::string& Title, const std::string& Text);

    /// @copydoc GenericMisc::GetCPUInformation
    static const FCPUInformation& GetCPUInformation();

    /// @copydoc GenericMisc::BaseAllocator
    static bool BaseAllocator(void* TargetMemory);

    /// @copydoc GenericMisc::LoadExternalModule
    static IExternalModule* LoadExternalModule(const std::string_view& ModuleName);

    /// @brief Return the XDG_CONFIG path
    static std::filesystem::path GetConfigPath();
};

// Helper to use the platform implementation
using FPlatformMisc = FLinuxMisc;
