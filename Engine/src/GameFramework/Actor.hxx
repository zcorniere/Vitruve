#pragma once

#include "GameFramework/Components/MeshComponent.hxx"

class ENGINE_API AActor : public RObject
{
    RTTI_DECLARE_TYPEINFO(AActor, RObject)
public:
    AActor();
    ~AActor();

    virtual void BeginPlay();
    virtual void EndPlay();

    virtual void Tick(double DeltaTime);

    RMeshComponent* GetMesh();

    RSceneComponent* GetRootComponent();
    void SetRootComponent(Ref<RSceneComponent> InRootComponent);

    void SetActorLocation(const FVector3& Location);
    void SetActorRotation(const FQuaternion& Rotation);
    void SetActorScale(const FVector3& Scale);
    void SetActorTransform(const FTransform& Transform);

    const FTransform& GetRelativeTransform() const;

    template <typename T>
    requires RTTI::IsRTTIApiAvailable<T>
    T* GetComponent()
    {
        return static_cast<T*>(FindComponent(T::TypeInfo::Id()));
    }

protected:
    virtual RObject* FindComponent(RTTI::FTypeId TypeId);

private:
    WeakRef<RSceneComponent> RootComponent = nullptr;
    Ref<RMeshComponent> MeshComponent = nullptr;
};
