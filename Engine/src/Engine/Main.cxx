#include "Engine/Core/Application.hxx"
#include "Engine/Core/Engine.hxx"
#include "Engine/Core/Log.hxx"
#include "Engine/Misc/CommandLine.hxx"
#include "Engine/Misc/Timer.hxx"
#include "Engine/Misc/Utils.hxx"
#include "RHI/GenericRHI.hxx"
#include "RHI/RHI.hxx"

#ifdef PLATFORM_WINDOWS
    #include <windows.h>
#endif    // PLATFORM_WINDOWS

DECLARE_LOGGER_CATEGORY(Core, LogEngine, Info)

FORCEINLINE int EngineLoop(IApplication* (*ApplicationEntryPoint)())
{
    GEngine = new FEngine;

    if (!GEngine->Initialisation())
    {
        return -1;
    }

    // Initialize the graphics RHI
    RHI::Create();
    GDynamicRHI->Init();

    IApplication* const Application = ApplicationEntryPoint();
    check(Application);
    if (!Application->OnEngineInitialization())
    {
        return -1;
    }

    GDynamicRHI->PostInit();

    int ExitStatus = 0;
    double DeltaTime = 0.0f;
    FrameLimiter Limiter;
    while (!Utils::HasRequestedExit(ExitStatus) || GEngine->ShouldExit())
    {
        VIT_PROFILE_FUNC("Engine Tick")
        Limiter.BeginFrame();

        GEngine->PreTick();
        RHI::BeginFrame();

        Application->Tick(DeltaTime);

        if (WeakRef<RWorld> World = GEngine->GetWorld())
        {
            World->Tick(DeltaTime);
        }

        // Tick the RHI
        RHI::Tick(DeltaTime);

        GEngine->PostTick();

        // End the frame on the RHI side
        RHI::EndFrame();
        RHI::FlushDeletionQueue(true);

        DeltaTime = Limiter.EndFrame();
        // Must be on the last line of the engine loop
        VIT_PROFILE_MARK_FRAME
    }
    // Only destroy if the return value is ok
    if (ExitStatus == 0)
    {
        RHI::RHIWaitUntilIdle();
        GEngine->OnApplicationDestruction();
        Application->OnEngineDestruction();
        RHI::Destroy();
        GEngine->Destroy();
    }

    delete Application;
    delete GEngine;

    return ExitStatus;
}

ENGINE_API int EngineMain(const int ac, const char* const* av, IApplication* (*ApplicationEntryPoint)())
{
    FCommandLine::Set(ac, av);

    FPlatform::Initialize();
    if (FCommandLine::Param("-waitfordebugger"))
    {
        while (!FPlatform::isDebuggerPresent())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        PLATFORM_BREAK();
    }

    int GuardedReturnValue = 0;
    {
        FLog Log;
        GuardedReturnValue = EngineLoop(ApplicationEntryPoint);

        // Make sure no RObjects are left undestroyed
        // Not strictly necessary, but this precaution don't hurt ¯\_(ツ)_/¯
        check(RObjectUtils::AreThereAnyLiveObject() == false);
    }
    FPlatform::Deinitialize();
    return GuardedReturnValue;
}
