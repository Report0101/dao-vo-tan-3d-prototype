#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/TrajectoryTypes.h"
#include "CultivatorLocomotion.generated.h"

/** Cosmetic locomotion reads local CharacterMovement only; never drives simulation outcomes. */
UCLASS(Transient, Blueprintable)
class DAOVOTAN3D_API UCultivatorLocomotion : public UAnimInstance
{
    GENERATED_BODY()
  public:
    UCultivatorLocomotion();
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
    UPROPERTY(BlueprintReadOnly) bool bAirborne = false;
    UPROPERTY(BlueprintReadOnly) bool bLanding = false;
    UPROPERTY(BlueprintReadOnly) bool bMoving = false;
    UPROPERTY(BlueprintReadOnly) bool bRunning = false;
    UPROPERTY(BlueprintReadOnly) bool bTurning = false;
    UPROPERTY(BlueprintReadOnly) FVector LocalAcceleration = FVector::ZeroVector;
    UPROPERTY(BlueprintReadOnly) float Speed = 0;
    UPROPERTY(BlueprintReadOnly) float LandingAge = 0;
    UPROPERTY(BlueprintReadOnly) FString SelectedDatabase;
    UPROPERTY(BlueprintReadOnly) FString SelectedAnimation;
    UPROPERTY(BlueprintReadOnly) int32 PoseSelections = 0;
    // Read-only diagnostics for landing/action recovery and pose sampling.
    UPROPERTY(BlueprintReadOnly) float SelectedSequenceTime = 0;
    UPROPERTY(BlueprintReadOnly) float ActionSequenceTime = 0;
    UPROPERTY(BlueprintReadOnly) float ActionBlendWeight = 0;
    UPROPERTY() TObjectPtr<class UPoseSearchDatabase> Database;
    UPROPERTY() TObjectPtr<class UChooserTable> Chooser;
    UPROPERTY() TObjectPtr<class UAnimSequence> ActionClip;
    UPROPERTY() TObjectPtr<class UAnimSequence> CarryClip;
    bool bHasSword = false;
    bool bActionLoop = true;
    bool bActionActive = false;
    bool bSwordAction = false;
    float ActionBlendTarget = 1;
    FTransformTrajectory Trajectory;
    TSet<FString> SeenDatabases;
    bool UsesCosmeticRootMotion() const
    {
        return RootMotionMode == ERootMotionMode::IgnoreRootMotion;
    }

  protected:
    virtual FAnimInstanceProxy *CreateAnimInstanceProxy() override;
    virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy *Proxy) override;

  private:
    float PreviousYaw = 0;
    bool bWasAirborne = false;
    FTransformTrajectory RawTrajectory;
    float DesiredYawLastUpdate = 0;
    float TurnHoldRemaining = 0;
    float LandingElapsed = 0;
};
