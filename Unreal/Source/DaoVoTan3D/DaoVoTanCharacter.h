#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DaoVoTanCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;

UCLASS()
class DAOVOTAN3D_API ADaoVoTanCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ADaoVoTanCharacter();

protected:
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
    UPROPERTY(VisibleAnywhere, Category="Visual")
    UStaticMeshComponent* PrototypeBody;

    UPROPERTY(VisibleAnywhere, Category="Camera")
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, Category="Camera")
    UCameraComponent* FollowCamera;

    void MoveForward(float Value);
    void MoveRight(float Value);
    void Turn(float Value);
    void LookUp(float Value);
    void StartSprint();
    void StopSprint();

    float WalkSpeed = 450.0f;
    float SprintSpeed = 750.0f;
};
