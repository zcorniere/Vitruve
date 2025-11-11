#pragma once

#include "GameFramework/Actor.hxx"

#include "GameFramework/Components/CameraComponent.hxx"

class ENGINE_API ACameraActor : public AActor
{
    RTTI_DECLARE_TYPEINFO(ACameraActor, AActor)

public:
    ACameraActor();
    virtual ~ACameraActor();

protected:
    RObject* FindComponent(RTTI::FTypeId TypeId) override;

private:
    Ref<RCameraComponent<float>> CameraComponent = nullptr;
};
