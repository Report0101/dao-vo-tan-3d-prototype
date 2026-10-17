#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GateCharacter.generated.h"
UCLASS()
class DAOVOTAN3D_API AGateCharacter : public ACharacter
{
    GENERATED_BODY()
  public:
    AGateCharacter();
    virtual void BeginPlay() override;
    virtual void Tick(float Dt) override;
    virtual void SetupPlayerInputComponent(UInputComponent *Input) override;
    UPROPERTY(VisibleAnywhere) TObjectPtr<class USpringArmComponent> Boom;
    UPROPERTY(VisibleAnywhere) TObjectPtr<class UCameraComponent> Camera;
    UPROPERTY(VisibleAnywhere) TObjectPtr<class UAppearanceComponent> Appearance;
    UPROPERTY(VisibleAnywhere) TObjectPtr<class UPresentationActions> Actions;
    UPROPERTY(VisibleAnywhere) TObjectPtr<class UAuraComponent> Aura;
    UPROPERTY(VisibleAnywhere) TObjectPtr<class UPortraitComponent> Portrait;
    UPROPERTY(EditAnywhere) TObjectPtr<class UPresentationRecipe> Recipe;
    UPROPERTY(VisibleAnywhere) TObjectPtr<class USceneComponent> WeaponSocket;
    UPROPERTY(VisibleAnywhere) TObjectPtr<class UStaticMeshComponent> Blade;
    UPROPERTY(VisibleAnywhere) TObjectPtr<class UStaticMeshComponent> Guard;
    UPROPERTY(VisibleAnywhere) TObjectPtr<class UTextRenderComponent> Nameplate;
    class UInputMappingContext *GetPresentationInputContext() const
    {
        return InputContext;
    }
    void CycleAura();
    void ToggleMeditation();
    void SwordAction();
    void ToggleRun();
    void ToggleStrafe();
    bool IsStrafing() const
    {
        return bStrafing;
    }
    void TryJump();
    /** One project-owned hand frame for the +Z-blade jian, shared by every pose. */
    static FTransform JianGripTransform();

  private:
    void MoveForward(const struct FInputActionValue &Value);
    void MoveRight(const struct FInputActionValue &Value);
    void LookYaw(const struct FInputActionValue &Value);
    void LookPitch(const struct FInputActionValue &Value);
    int32 AuraIndex = 0;
    bool bRun = false;
    bool bStrafing = false;
    UPROPERTY() TObjectPtr<class UInputMappingContext> InputContext;
    UPROPERTY() TArray<TObjectPtr<class UInputAction>> InputActions;
};
