#pragma once

#include "ECS/World.hxx"
#include <Engine/Core/Application.hxx>

class EditorApplication : public FBaseApplication
{
    RTTI_DECLARE_TYPEINFO(EditorApplication, FBaseApplication)
public:
    EditorApplication();
    ~EditorApplication();

    bool OnEngineInitialization() override;
    void OnEngineDestruction() override;

    void Tick(const double DeltaTime) override;

private:
    Ref<ecs::RWorld> World = nullptr;
    ecs::FEntity CameraEntity;
};

IApplication* CreateApplication();
