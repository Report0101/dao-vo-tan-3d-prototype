#include "PresentationActions.h"
#include "Animation/AnimSequence.h"
#include "AuraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GateCharacter.h"
#include "CultivatorLocomotion.h"
UPresentationActions::UPresentationActions()
{
    PrimaryComponentTick.bCanEverTick = true;
}
void UPresentationActions::BeginPlay()
{
    Super::BeginPlay();
    Sword = LoadObject<UAnimSequence>(nullptr, TEXT("/Game/DaoVoTan/Visual11/AS_SwordAttackA"));
    Enter = LoadObject<UAnimSequence>(nullptr, TEXT("/Game/DaoVoTan/Animation/AS_MeditationEnter"));
    Loop = LoadObject<UAnimSequence>(nullptr, TEXT("/Game/DaoVoTan/Animation/AS_MeditationLoop"));
    Exit = LoadObject<UAnimSequence>(nullptr, TEXT("/Game/DaoVoTan/Animation/AS_MeditationExit"));
    SetPhase(EPresentationPhase::Locomotion);
}
void UPresentationActions::ResetToLocomotionPresentation()
{
    // An explicit snapshot/receipt reset retains its accepted grounded-preview
    // contract, including when a fresh pawn has not yet settled onto the floor.
    // Ordinary sword transitions below preserve an ongoing fall.
    if (auto *C = Cast<AGateCharacter>(GetOwner()))
        C->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    SetPhase(EPresentationPhase::Locomotion);
}
void UPresentationActions::SetPhase(EPresentationPhase Next)
{
    auto *C = Cast<AGateCharacter>(GetOwner());
    if (!C)
        return;
    Phase = Next;
    PhaseAge = 0;
    auto *Move = C->GetCharacterMovement();
    if (IsMeditating())
    {
        Move->StopMovementImmediately();
        Move->DisableMovement();
    }
    else if (Move->MovementMode == MOVE_None)
        Move->SetMovementMode(MOVE_Walking);
    UAnimationAsset *Clip = nullptr;
    bool Repeat = true;
    switch (Phase)
    {
    case EPresentationPhase::Sword:
        Clip = Sword;
        Repeat = false;
        break;
    case EPresentationPhase::MeditationEnter:
        Clip = Enter;
        Repeat = false;
        break;
    case EPresentationPhase::MeditationLoop:
        Clip = Loop;
        break;
    case EPresentationPhase::MeditationExit:
        Clip = Exit;
        Repeat = false;
        break;
    default:
        break;
    }
    if (!Cast<UCultivatorLocomotion>(C->GetMesh()->GetAnimInstance()))
        C->GetMesh()->SetAnimInstanceClass(UCultivatorLocomotion::StaticClass());
    if (auto *Animation = Cast<UCultivatorLocomotion>(C->GetMesh()->GetAnimInstance()))
    {
        if (Phase != EPresentationPhase::Locomotion)
        {
            Animation->ActionClip = Cast<UAnimSequence>(Clip);
            Animation->bActionLoop = Repeat;
            Animation->bSwordAction = Phase == EPresentationPhase::Sword;
            Animation->ActionBlendTarget = 1;
        }
        Animation->bActionActive = Phase != EPresentationPhase::Locomotion;
    }
    C->Aura->SetEmitting(Phase == EPresentationPhase::MeditationLoop);
    UE_LOG(LogTemp, Display, TEXT("GATE1 phase actor=%s phase=%d (presentation only)"),
           *C->GetName(), int32(Phase));
}
void UPresentationActions::StartMeditationPresentation()
{
    auto *C = Cast<AGateCharacter>(GetOwner());
    if (Phase == EPresentationPhase::Locomotion && C && !C->GetCharacterMovement()->IsFalling())
        SetPhase(EPresentationPhase::MeditationEnter);
}
void UPresentationActions::StopMeditationPresentation()
{
    if (Phase == EPresentationPhase::MeditationEnter || Phase == EPresentationPhase::MeditationLoop)
        SetPhase(EPresentationPhase::MeditationExit);
}
void UPresentationActions::PlaySwordPresentation()
{
    if (Phase == EPresentationPhase::Locomotion)
        SetPhase(EPresentationPhase::Sword);
}
void UPresentationActions::TickComponent(float Dt, ELevelTick Tick, FActorComponentTickFunction *Fn)
{
    Super::TickComponent(Dt, Tick, Fn);
    auto *C = Cast<AGateCharacter>(GetOwner());
    if (!C)
        return;
    PhaseAge += Dt;
    if (Phase == EPresentationPhase::Sword && Sword)
        if (auto *Animation = Cast<UCultivatorLocomotion>(C->GetMesh()->GetAnimInstance()))
            Animation->ActionBlendTarget =
                1.f - FMath::SmoothStep(.82f, 1.f, PhaseAge / Sword->GetPlayLength());
    if (Phase == EPresentationPhase::MeditationEnter && PhaseAge >= 1.2f)
        SetPhase(EPresentationPhase::MeditationLoop);
    if (Phase == EPresentationPhase::MeditationExit && PhaseAge >= 1.2f)
        SetPhase(EPresentationPhase::Locomotion);
    if (Phase == EPresentationPhase::Sword && PhaseAge >= (Sword ? Sword->GetPlayLength() : 1.f))
        SetPhase(EPresentationPhase::Locomotion);
}
