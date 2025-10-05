#pragma once

class IModuleInterface : public FNamedClass
{
    RTTI_DECLARE_TYPEINFO(IModuleInterface, FNamedClass)
public:
    virtual ~IModuleInterface() = default;

    /// Called when the module is loaded
    virtual void StartupModule() = 0;

    /// Called when the module is unloaded
    virtual void ShutdownModule() = 0;
};

class IRHIModule : public IModuleInterface
{
    RTTI_DECLARE_TYPEINFO(IRHIModule, IModuleInterface)
public:
    /// Create the RHI instance
    virtual class FGenericRHI* CreateRHI() = 0;
};
