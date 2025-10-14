#pragma once

#include "AssetRegistry/AssetRegistry.hxx"
#include "Engine/GameFramework/World.hxx"
#include "Engine/Threading/ThreadPool.hxx"

extern ENGINE_API class FEngine* GEngine;

class ENGINE_API FEngine
{
public:
    FEngine();
    ~FEngine();

    FThreadPool& GetThreadPool()
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

    Ref<RWorld> CreateWorld();
    void SetWorld(Ref<RWorld> World);
    Ref<RWorld> GetWorld() const;

public:
    std::unique_ptr<FAssetRegistry> AssetRegistry;
    FThreadPool m_ThreadPool;

private:
    Ref<RWorld> LoadedWorld = nullptr;
};
