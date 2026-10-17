#pragma once
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "PortraitComponent.generated.h"
/** Captures the actual recipe-rendered actor, not an independently selected portrait asset. */
UCLASS(ClassGroup = Presentation)
class DAOVOTAN3D_API UPortraitComponent : public UActorComponent
{
    GENERATED_BODY()
  public:
    UPortraitComponent();
    class UTextureRenderTarget2D *RequestPortrait();
    virtual void TickComponent(float Dt, ELevelTick Tick, FActorComponentTickFunction *Fn) override;

  private:
    UPROPERTY(Transient) TObjectPtr<class USceneCaptureComponent2D> Capture;
    UPROPERTY(Transient) TObjectPtr<class UTextureRenderTarget2D> Target;
    UPROPERTY(Transient) TObjectPtr<class UPointLightComponent> KeyLight;
    UPROPERTY(Transient) TObjectPtr<class UStaticMeshComponent> Backdrop;
    float Age = 0;
    void Refresh();
};
