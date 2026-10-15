#include "GateCharacter.h"
#include "AppearanceComponent.h"
#include "AuraComponent.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "PresentationActions.h"
AGateCharacter::AGateCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    GetCapsuleComponent()->InitCapsuleSize(38, 92);
    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0, 500, 0);
    GetCharacterMovement()->MaxWalkSpeed = 240;
    GetCharacterMovement()->bRunPhysicsWithNoController = true;
    GetCharacterMovement()->JumpZVelocity = 500;
    GetCharacterMovement()->AirControl = .3f;
    GetMesh()->SetRelativeLocation(FVector(0, 0, -92));
    GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));
    Boom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    Boom->SetupAttachment(RootComponent);
    Boom->TargetArmLength = 420;
    Boom->SocketOffset = FVector(0, 55, 65);
    Boom->bUsePawnControlRotation = true;
    Boom->bEnableCameraLag = true;
    Boom->CameraLagSpeed = 9;
    Boom->ProbeSize = 16;
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    Camera->SetupAttachment(Boom);
    Camera->FieldOfView = 75;
    Appearance = CreateDefaultSubobject<UAppearanceComponent>(TEXT("Appearance"));
    Actions = CreateDefaultSubobject<UPresentationActions>(TEXT("PresentationActions"));
    Aura = CreateDefaultSubobject<UAuraComponent>(TEXT("Aura"));
    Aura->SetupAttachment(RootComponent);
    WeaponSocket = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponSocket"));
    WeaponSocket->SetupAttachment(GetMesh(), TEXT("hand_r"));
    WeaponSocket->SetRelativeRotation(FRotator(0, 0, 90));
    Blade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwordBlade"));
    Blade->SetupAttachment(WeaponSocket);
    Blade->SetRelativeLocation(FVector(0, 0, 55));
    Blade->SetRelativeScale3D(FVector(.035f, .09f, .95f));
    Blade->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Guard = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwordGuard"));
    Guard->SetupAttachment(WeaponSocket);
    Guard->SetRelativeLocation(FVector(0, 0, 7));
    Guard->SetRelativeScale3D(FVector(.08f, .30f, .04f));
    Guard->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Nameplate = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Nameplate"));
    Nameplate->SetupAttachment(RootComponent);
    Nameplate->SetRelativeLocation(FVector(0, 0, 135));
    Nameplate->SetHorizontalAlignment(EHTA_Center);
    Nameplate->SetWorldSize(20);
    Nameplate->SetTextRenderColor(FColor(221, 209, 168));
}
void AGateCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (!Recipe)
        Recipe = LoadObject<UPresentationRecipe>(nullptr, TEXT("/Game/DaoVoTan/Data/DA_Player"));
    Appearance->Apply(Recipe);
    Blade->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")));
    Guard->SetStaticMesh(Blade->GetStaticMesh());
    Blade->SetMaterial(0, LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/DaoVoTan/Materials/M_Steel")));
    Guard->SetMaterial(0, LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/DaoVoTan/Materials/M_Gold")));
    Blade->SetVisibility(Recipe && Recipe->bSword);
    Guard->SetVisibility(Recipe && Recipe->bSword);
    if (Recipe)
    {
        Aura->Configure(Recipe->Aura);
        Nameplate->SetText(Recipe->DisplayName);
    }
    Nameplate->SetHiddenInGame(IsPlayerControlled());
}
void AGateCharacter::Tick(float Dt)
{
    Super::Tick(Dt);
    if (Actions->IsMeditating())
        Boom->TargetArmLength = FMath::FInterpTo(Boom->TargetArmLength, 330, Dt, 2);
    else
        Boom->TargetArmLength = FMath::FInterpTo(Boom->TargetArmLength, 420, Dt, 2);
    if (auto *PC = GetWorld()->GetFirstPlayerController())
        if (PC->PlayerCameraManager)
            Nameplate->SetWorldRotation(
                (PC->PlayerCameraManager->GetCameraLocation() - Nameplate->GetComponentLocation())
                    .Rotation());
}
void AGateCharacter::SetupPlayerInputComponent(UInputComponent *Input)
{
    Super::SetupPlayerInputComponent(Input);
    auto *E = Cast<UEnhancedInputComponent>(Input);
    auto *PC = Cast<APlayerController>(GetController());
    if (!E || !PC)
        return;
    InputContext = NewObject<UInputMappingContext>(this);
    auto Axis = [&](const TCHAR *Name, FKey Key, float Scale, auto Method)
    {
        auto *A = NewObject<UInputAction>(this);
        A->ValueType = EInputActionValueType::Axis1D;
        InputActions.Add(A);
        auto &Mapping = InputContext->MapKey(A, Key);
        if (Scale < 0)
            Mapping.Modifiers.Add(NewObject<UInputModifierNegate>(this));
        E->BindAction(A, ETriggerEvent::Triggered, this, Method);
    };
    Axis(TEXT("Forward"), EKeys::W, 1, &AGateCharacter::MoveForward);
    Axis(TEXT("Back"), EKeys::S, -1, &AGateCharacter::MoveForward);
    // Negative digital axes use Enhanced Input negate modifiers.

    Axis(TEXT("Right"), EKeys::D, 1, &AGateCharacter::MoveRight);
    Axis(TEXT("Left"), EKeys::A, -1, &AGateCharacter::MoveRight);
    Axis(TEXT("Yaw"), EKeys::MouseX, 1, &AGateCharacter::LookYaw);
    Axis(TEXT("Pitch"), EKeys::MouseY, 1, &AGateCharacter::LookPitch);
    auto Button = [&](FKey Key, auto Method)
    {
        auto *A = NewObject<UInputAction>(this);
        InputActions.Add(A);
        InputContext->MapKey(A, Key);
        E->BindAction(A, ETriggerEvent::Started, this, Method);
    };
    Button(EKeys::SpaceBar, &AGateCharacter::TryJump);
    Button(EKeys::LeftShift, &AGateCharacter::ToggleRun);
    Button(EKeys::LeftMouseButton, &AGateCharacter::SwordAction);
    Button(EKeys::E, &AGateCharacter::ToggleMeditation);
    Button(EKeys::M, &AGateCharacter::ToggleMeditation);
    Button(EKeys::One, &AGateCharacter::CycleAura);
    if (auto *S = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        S->AddMappingContext(InputContext, 0);
}
void AGateCharacter::MoveForward(const FInputActionValue &V)
{
    if (!Actions->IsMeditating())
        AddMovementInput(
            FRotationMatrix(FRotator(0, Controller->GetControlRotation().Yaw, 0)).GetUnitAxis(EAxis::X),
            V.Get<float>());
}
void AGateCharacter::MoveRight(const FInputActionValue &V)
{
    if (!Actions->IsMeditating())
        AddMovementInput(
            FRotationMatrix(FRotator(0, Controller->GetControlRotation().Yaw, 0)).GetUnitAxis(EAxis::Y),
            V.Get<float>());
}
void AGateCharacter::LookYaw(const FInputActionValue &V)
{
    AddControllerYawInput(V.Get<float>());
}
void AGateCharacter::LookPitch(const FInputActionValue &V)
{
    AddControllerPitchInput(-V.Get<float>());
}
void AGateCharacter::TryJump()
{
    if (!Actions->IsMeditating())
        Jump();
}
void AGateCharacter::ToggleRun()
{
    bRun = !bRun;
    GetCharacterMovement()->MaxWalkSpeed = bRun ? 600 : 240;
}
void AGateCharacter::SwordAction()
{
    Actions->PlaySwordPresentation();
}
void AGateCharacter::ToggleMeditation()
{
    if (Actions->IsMeditating())
        Actions->StopMeditationPresentation();
    else
        Actions->StartMeditationPresentation();
}
void AGateCharacter::CycleAura()
{
    AuraIndex = (AuraIndex + 1) % 3;
    FAuraStyle S = Recipe ? Recipe->Aura : FAuraStyle();
    if (AuraIndex == 1)
    {
        S.Color = FLinearColor(1, .55f, .12f);
        S.Radius = 115;
        S.Intensity = 2;
        S.Density = 56;
        S.PulseSpeed = 2;
    }
    if (AuraIndex == 2)
    {
        S.Color = FLinearColor(.5f, .25f, 1);
        S.Radius = 145;
        S.Intensity = 3;
        S.Density = 80;
        S.PulseSpeed = 3.2f;
    }
    Aura->Configure(S);
}
