#pragma once

#include "AssetRegistry/AssetRegistry.hxx"
#include "Engine/GameFramework/World.hxx"
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

    ENGINE_API Ref<RWorld> CreateWorld();
    ENGINE_API void SetWorld(Ref<RWorld> World);
    ENGINE_API Ref<RWorld> GetWorld() const;

public:
    std::unique_ptr<FAssetRegistry> AssetRegistry;
    FThreadPool m_ThreadPool;

private:
    Ref<RWorld> LoadedWorld = nullptr;
};
