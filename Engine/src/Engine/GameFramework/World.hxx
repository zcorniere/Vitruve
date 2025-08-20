#pragma once

#include "Engine/Math/Transform.hxx"
#include "Engine/Misc/Delegate.hxx"

class AActor;
class RSceneComponent;
class RRHIScene;

class RWorld : public RObject
{
    RTTI_DECLARE_TYPEINFO(RWorld, RObject)
public:
    RWorld();
    ~RWorld();

    virtual void SetName(std::string_view InName) override;

    template <typename T>
    requires std::derived_from<T, AActor>
    Ref<T> CreateActor(std::string Name, FTransform Transform)
    {
        VIT_PROFILE_FUNC();
        Ref<T> Actor = Ref<T>::CreateNamed(Name);
        AddToWorld(Actor);

        Actor->SetActorTransform(Transform);
        return Actor;
    }

    void AddToWorld(Ref<AActor> Actor);
    void RemoveFromWorld(Ref<AActor> Actor);

    void Tick(double DeltaTime);

    Ref<RRHIScene> GetScene() const;
    FVector3 GetGravity() const;

private:
    void UpdateActorLocation(uint64 ID, RSceneComponent* const Component);
    void HandleActorTick(AActor* const Actor, double DeltaTime);

public:
    TDelegate<void(AActor*)> OnActorAddedToWorld;
    TDelegate<void(AActor*)> OnActorRemovedFromWorld;

private:
    TArray<Ref<AActor>> Actors;

    FVector3 Gravity = {0, 0, -9.807};    // Default gravity vector

    Ref<RRHIScene> Scene = nullptr;
};
