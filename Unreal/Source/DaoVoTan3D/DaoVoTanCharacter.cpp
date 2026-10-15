#include "DaoVoTanCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

ADaoVoTanCharacter::ADaoVoTanCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 620.0f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 360.0f;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 80.0f));

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
}

void ADaoVoTanCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ADaoVoTanCharacter::MoveForward);
    PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ADaoVoTanCharacter::MoveRight);
    PlayerInputComponent->BindAxis(TEXT("Turn"), this, &ADaoVoTanCharacter::Turn);
    PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ADaoVoTanCharacter::LookUp);

    PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &ACharacter::StopJumping);
    PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Pressed, this, &ADaoVoTanCharacter::StartSprint);
    PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Released, this, &ADaoVoTanCharacter::StopSprint);
}

void ADaoVoTanCharacter::MoveForward(float Value)
{
    if (!Controller || FMath::IsNearlyZero(Value))
    {
        return;
    }

    const FRotator ControlRotation = Controller->GetControlRotation();
    const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
    AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), Value);
}

void ADaoVoTanCharacter::MoveRight(float Value)
{
    if (!Controller || FMath::IsNearlyZero(Value))
    {
        return;
    }

    const FRotator ControlRotation = Controller->GetControlRotation();
    const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
    AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), Value);
}

void ADaoVoTanCharacter::Turn(float Value)
{
    AddControllerYawInput(Value);
}

void ADaoVoTanCharacter::LookUp(float Value)
{
    AddControllerPitchInput(Value);
}

void ADaoVoTanCharacter::StartSprint()
{
    GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void ADaoVoTanCharacter::StopSprint()
{
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}
