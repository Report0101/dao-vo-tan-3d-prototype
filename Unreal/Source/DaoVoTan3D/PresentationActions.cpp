#include "PresentationActions.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "AuraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GateCharacter.h"
UPresentationActions::UPresentationActions()
{
    PrimaryComponentTick.bCanEverTick = true;
}
void UPresentationActions::BeginPlay()
{
    Super::BeginPlay();
    Locomotion = LoadObject<UAnimationAsset>(
        nullptr, TEXT("/Game/Characters/Mannequins/Anims/Unarmed/BS_Idle_Walk_Run"));
    Fall = LoadObject<UAnimSequence>(nullptr,
                                     TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Jump/MM_Fall_Loop"));
    Sword = LoadObject<UAnimSequence>(nullptr,
                                      TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_01"));
    Enter = LoadObject<UAnimSequence>(nullptr, TEXT("/Game/DaoVoTan/Animation/AS_MeditationEnter"));
    Loop = LoadObject<UAnimSequence>(nullptr, TEXT("/Game/DaoVoTan/Animation/AS_MeditationLoop"));
    Exit = LoadObject<UAnimSequence>(nullptr, TEXT("/Game/DaoVoTan/Animation/AS_MeditationExit"));
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
    else
        Move->SetMovementMode(MOVE_Walking);
    UAnimationAsset *Clip = Locomotion;
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
    C->GetMesh()->PlayAnimation(Clip, Repeat);
    C->Aura->SetEmitting(Phase == EPresentationPhase::MeditationLoop);
    UE_LOG(LogTemp, Display, TEXT("GATE1 phase actor=%s phase=%d (presentation only)"), *C->GetName(),
           int32(Phase));
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
    if (Phase == EPresentationPhase::MeditationEnter && PhaseAge >= 1.2f)
        SetPhase(EPresentationPhase::MeditationLoop);
    if (Phase == EPresentationPhase::MeditationExit && PhaseAge >= 1.2f)
        SetPhase(EPresentationPhase::Locomotion);
    if (Phase == EPresentationPhase::Sword && PhaseAge >= (Sword ? Sword->GetPlayLength() : 1.f))
        SetPhase(EPresentationPhase::Locomotion);
    if (Phase == EPresentationPhase::Locomotion)
    {
        bool Air = C->GetCharacterMovement()->IsFalling();
        if (Air != bFalling)
        {
            bFalling = Air;
            C->GetMesh()->PlayAnimation(Air ? Cast<UAnimationAsset>(Fall) : Locomotion.Get(), true);
        }
        if (!Air)
            if (auto *Instance = C->GetMesh()->GetSingleNodeInstance())
                Instance->SetBlendSpacePosition(FVector(C->GetVelocity().Size2D(), 0, 0));
    }
}
