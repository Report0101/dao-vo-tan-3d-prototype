#pragma once
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "PresentationActions.generated.h"
UENUM(BlueprintType)
enum class EPresentationPhase : uint8
{
    Locomotion,
    Sword,
    MeditationEnter,
    MeditationLoop,
    MeditationExit
};
UCLASS(ClassGroup = Presentation, meta = (BlueprintSpawnableComponent))
class DAOVOTAN3D_API UPresentationActions : public UActorComponent
{
    GENERATED_BODY()
  public:
    UPresentationActions();
    void ResetToLocomotionPresentation();
    UFUNCTION(BlueprintCallable) void StartMeditationPresentation();
    UFUNCTION(BlueprintCallable) void StopMeditationPresentation();
    UFUNCTION(BlueprintCallable) void PlaySwordPresentation();
    EPresentationPhase GetPhase() const
    {
        return Phase;
    }
    bool IsMeditating() const
    {
        return Phase == EPresentationPhase::MeditationEnter ||
               Phase == EPresentationPhase::MeditationLoop ||
               Phase == EPresentationPhase::MeditationExit;
    }
    virtual void BeginPlay() override;
    virtual void TickComponent(float Dt, ELevelTick Tick, FActorComponentTickFunction *Fn) override;

  private:
    EPresentationPhase Phase = EPresentationPhase::Locomotion;
    float PhaseAge = 0;
    void SetPhase(EPresentationPhase Next);
    UPROPERTY() TObjectPtr<class UAnimSequence> Sword;
    UPROPERTY() TObjectPtr<class UAnimSequence> Enter;
    UPROPERTY() TObjectPtr<class UAnimSequence> Loop;
    UPROPERTY() TObjectPtr<class UAnimSequence> Exit;
};
