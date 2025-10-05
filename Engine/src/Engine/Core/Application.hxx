#pragma once

#include "Engine/Core/Events/ApplicationEvent.hxx"
#include "Engine/Core/RHI/Resources/RHIViewport.hxx"
#include "Engine/Core/Window.hxx"

extern class IApplication* GApplication;

class IApplication : public RTTI::FEnable
{
    RTTI_DECLARE_TYPEINFO(IApplication)
public:
    virtual ~IApplication()
    {
    }

    virtual RWindow* GetMainWindow() = 0;

    /// Called when the Engine created
    virtual bool OnEngineInitialization() = 0;
    /// Called when the Engine is destroyed
    virtual void OnEngineDestruction() = 0;

    /// Called once per frame
    /// @param DeltaTime the time elapsed since last frame
    virtual void Tick(const double DeltaTime) = 0;
};

class ENGINE_API FBaseApplication : public IApplication
{
    RTTI_DECLARE_TYPEINFO(FBaseApplication, IApplication)
public:
    FBaseApplication();
    virtual ~FBaseApplication();

    virtual RWindow* GetMainWindow() override
    {
        return MainWindow.Raw();
    }

    virtual bool OnEngineInitialization() override;
    virtual void OnEngineDestruction() override;

    virtual void Tick(const double DeltaTime) override;

private:
    virtual void WindowEventHandler(FEvent& Event);

    virtual bool OnWindowResize(FWindowResizeEvent& e);
    virtual bool OnWindowMinimize(FWindowMinimizeEvent& e);
    virtual bool OnWindowClose(FWindowCloseEvent& e);

protected:
    bool bShouldExit = false;

    Ref<RWindow> MainWindow;
    Ref<RRHIViewport> MainViewport;
};
