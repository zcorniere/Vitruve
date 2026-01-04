#pragma once

#include "AssetRegistry/AssetRegistry.hxx"
#include "ECS/World.hxx"
#include "Engine/Threading/ThreadPool.hxx"

extern ENGINE_API class FEngine* GEngine;

class FEngine
{
public:
    FEngine();
    ~FEngine();

    ENGINE_API FThreadPool& GetThreadPool()
    {
        return m_ThreadPool;
    }

public:
    bool ShouldExit() const;

    bool Initialisation();
    void OnApplicationDestruction();
    void Destroy();

    void PreTick();
    void PostTick();

    ENGINE_API void SetWorld(Ref<ecs::RWorld> World);
    ENGINE_API Ref<ecs::RWorld> GetWorld() const;

public:
    FAssetRegistry AssetRegistry;
    FThreadPool m_ThreadPool;

private:
    Ref<ecs::RWorld> LoadedWorld = nullptr;
};
