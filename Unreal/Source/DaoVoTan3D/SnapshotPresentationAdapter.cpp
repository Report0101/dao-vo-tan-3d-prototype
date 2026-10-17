#include "SnapshotPresentationAdapter.h"
#include "AppearanceComponent.h"
#include "AuraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GateCharacter.h"
#include "HAL/FileManager.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "PresentationActions.h"
#include "PresentationRecipe.h"
USnapshotPresentationAdapter::USnapshotPresentationAdapter()
{
    PrimaryComponentTick.bCanEverTick = true;
}
void USnapshotPresentationAdapter::SetSlots(AGateCharacter *Player,
                                            const TArray<AGateCharacter *> &NPCs)
{
    Slots.Empty();
    Slots.Add(Player);
    for (int32 I = 0; I < FMath::Min(2, NPCs.Num()); ++I)
        Slots.Add(NPCs[I]);
    FParse::Value(FCommandLine::Get(), TEXT("SnapshotFile="), FilePath);
    if (!FilePath.IsEmpty())
        LoadSnapshotFile(FilePath, true);
}
AGateCharacter *USnapshotPresentationAdapter::ActorFor(const FString &Id) const
{
    const auto *A = Bindings.Find(Id);
    return A ? A->Get() : nullptr;
}
const FSnapshotActorState *USnapshotPresentationAdapter::StateFor(const AGateCharacter *Actor) const
{
    for (const auto &S : Current.Actors)
        if (ActorFor(S.ActorId) == Actor)
            return &S;
    return nullptr;
}
void USnapshotPresentationAdapter::ApplyAppearance(AGateCharacter *Actor, const FString &Seed)
{
    const uint32 Token = uint32(FCString::Strtoui64(*Seed.Left(8), nullptr, 16));
    const TCHAR *Paths[] = {TEXT("/Game/DaoVoTan/Data/DA_Player"),
                            TEXT("/Game/DaoVoTan/Data/DA_NPC_A"),
                            TEXT("/Game/DaoVoTan/Data/DA_NPC_B")};
    auto *Base = LoadObject<UPresentationRecipe>(nullptr, Paths[Token % 3]);
    auto *Recipe = DuplicateObject<UPresentationRecipe>(Base, this);
    Recipe->RealmLabel = FText::FromString(TEXT("Presentation Alias"));
    Recipe->ApplyAppearanceSeed(Seed);
    Actor->Recipe = Recipe;
    Actor->Appearance->Apply(Recipe);
    Actor->Aura->Configure(Recipe->Aura);
    Actor->Blade->SetVisibility(Recipe->bSword);
    Actor->Guard->SetVisibility(false); // Guard is part of the authored jian mesh.
    Actor->Actions->ResetToLocomotionPresentation();
}
bool USnapshotPresentationAdapter::ApplyJson(const FString &Json, bool bResync, FString &Error)
{
    const double Start = FPlatformTime::Seconds();
    FWorldPresentationSnapshot Next;
    auto Reject = [&](const TCHAR *Why)
    {
        Error = Why;
        ++RejectedCount;
        return false;
    };
    if (!SnapshotV1::Parse(Json, Next, Error))
    {
        ++RejectedCount;
        return false;
    }
    LastParseMs = (FPlatformTime::Seconds() - Start) * 1000;
    const double ApplyStart = FPlatformTime::Seconds();
    if (Next.Actors.Num() > 3 || Slots.Num() != 3)
        return Reject(
            TEXT("Adapter requires three prepared fixture slots (max 3 projected actors)"));
    for (auto *Slot : {Slots[0].Get(), Slots[1].Get(), Slots[2].Get()})
        if (!IsValid(Slot))
            return Reject(TEXT("Fixture slot missing"));
    if (!Current.StreamId.IsEmpty() &&
        (Current.WorldId != Next.WorldId || Current.RulesetVersion != Next.RulesetVersion ||
         Current.SimulationVersion != Next.SimulationVersion))
        return Reject(TEXT("World/version mismatch; create a new adapter session"));
    const bool NewStream = Current.StreamId != Next.StreamId;
    if (NewStream)
    {
        if (!bResync || Next.Sequence != 1 || Next.Cues.Num() != 0 ||
            RetiredStreams.Contains(Next.StreamId))
            return Reject(TEXT("New stream needs explicit full resync, sequence 1 and no actions"));
    }
    else if (Next.Sequence <= Current.Sequence || Next.WorldTick < Current.WorldTick)
        return Reject(TEXT("Old sequence or backwards world tick"));
    if (!NewStream && bResync)
        return Reject(TEXT("Full resync requires a fresh stream"));
    TMap<FString, TObjectPtr<AGateCharacter>> Plan;
    TSet<AGateCharacter *> Used;
    if (!Next.ControlledActorId.IsEmpty())
    {
        Plan.Add(Next.ControlledActorId, Slots[0]);
        Used.Add(Slots[0]);
    }
    for (const auto &S : Next.Actors)
    {
        if (S.ActorId == Next.ControlledActorId)
            continue;
        auto *Old = ActorFor(S.ActorId);
        if (Old && Old != Slots[0] && !Used.Contains(Old))
        {
            Plan.Add(S.ActorId, Old);
            Used.Add(Old);
        }
    }
    for (const auto &S : Next.Actors)
    {
        if (Plan.Contains(S.ActorId))
            continue;
        bool Found = false;
        for (int32 I = 1; I < 3; ++I)
            if (!Used.Contains(Slots[I]))
            {
                Plan.Add(S.ActorId, Slots[I]);
                Used.Add(Slots[I]);
                Found = true;
                break;
            }
        if (!Found)
            return Reject(TEXT("Insufficient NPC fixture slots"));
    }
    int32 Fresh = 0;
    for (const auto &C : Next.Cues)
    {
        const FString Fingerprint = C.ActorId + TEXT("/") + LexToString(C.Tick);
        if (const FString *Old = SeenCues.Find(C.EventId))
        {
            if (*Old != Fingerprint)
                return Reject(TEXT("Conflicting replay of event identity"));
        }
        else
            ++Fresh;
    }
    if (SeenCues.Num() + Fresh > 4096)
        return Reject(TEXT("Session cue limit; review/reset adapter before continuing"));
    // Validation and complete binding plan succeeded. Only cosmetic objects are touched below.
    if (NewStream)
    {
        if (!Current.StreamId.IsEmpty())
            RetiredStreams.Add(Current.StreamId);
        Pending.Empty();
        Playing.Empty();
        for (auto Slot : Slots)
            Slot->Actions->ResetToLocomotionPresentation();
    }

    for (auto Slot : Slots)
        if (!Used.Contains(Slot))
        {
            Slot->Actions->ResetToLocomotionPresentation();
            Slot->SetActorHiddenInGame(true);
            Slot->SetActorEnableCollision(false);
        }
    for (const auto &S : Next.Actors)
    {
        auto *A = Plan[S.ActorId].Get();
        const auto *Old = StateFor(A);
        if (!Old || Old->ActorId != S.ActorId || Old->AppearanceSeed != S.AppearanceSeed)
            ApplyAppearance(A, S.AppearanceSeed);
        A->SetActorHiddenInGame(false);
        A->SetActorEnableCollision(true);
        A->Nameplate->SetText(FText::FromString((A == Slots[0]   ? TEXT("Player")
                                                 : A == Slots[1] ? TEXT("NPC A")
                                                                 : TEXT("NPC B")) +
                                                FString(TEXT(" [alias] / ")) + S.LifeStatus));
        if (Old && Old->LifeStatus == TEXT("dead") && S.LifeStatus != TEXT("dead"))
            A->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        if (S.LifeStatus == TEXT("dead"))
        {
            A->Actions->ResetToLocomotionPresentation();
            A->GetCharacterMovement()->DisableMovement();
            UE_LOG(LogTemp, Display, TEXT("SNAPSHOT dead state %s; no death animation implemented"),
                   *S.ActorId);
        }
    }
    Bindings = MoveTemp(Plan);
    Current = MoveTemp(Next);
    for (const auto &C : Current.Cues)
    {
        if (SeenCues.Contains(C.EventId))
        {
            ++DuplicateCueCount;
            continue;
        }
        SeenCues.Add(C.EventId, C.ActorId + TEXT("/") + LexToString(C.Tick));
        if (!CommandPreviewActors.Contains(C.ActorId))
            Pending.Add(C);
        ++UniqueCueCount;
    }
    LastApplyMs = (FPlatformTime::Seconds() - ApplyStart) * 1000;
    UE_LOG(LogTemp, Display,
           TEXT("SNAPSHOT accepted stream=%s sequence=%lld bindings=%d unique=%d duplicates=%d "
                "parse_ms=%.4f "
                "apply_ms=%.4f"),
           *Current.StreamId, Current.Sequence, Bindings.Num(), UniqueCueCount, DuplicateCueCount,
           LastParseMs, LastApplyMs);
    return true;
}
bool USnapshotPresentationAdapter::LoadSnapshotFile(const FString &Path, bool bResync)
{
    if (IFileManager::Get().FileSize(*Path) > 1024 * 1024)
        return false;
    FString Text, Error;
    if (!FFileHelper::LoadFileToString(Text, *Path))
        return false;
    bool Accepted = ApplyJson(Text, bResync, Error);
    if (Accepted)
        LastFileContent = Text;
    else
        UE_LOG(LogTemp, Display, TEXT("SNAPSHOT rejected: %s"), *Error);
    return Accepted;
}
void USnapshotPresentationAdapter::TickComponent(float Dt, ELevelTick Tick,
                                                 FActorComponentTickFunction *Fn)
{
    Super::TickComponent(Dt, Tick, Fn);
    PollAge += Dt;
    if (Slots.Num() == 3 && !IsValid(Slots[0]))
    {
        auto *PC = GetWorld()->GetFirstPlayerController();
        if (PC)
            Slots[0] = Cast<AGateCharacter>(PC->GetPawn());
    }
    if (!FilePath.IsEmpty() && PollAge > .5f)
    {
        PollAge = 0;
        FString Text;
        if (IFileManager::Get().FileSize(*FilePath) <= 1024 * 1024 &&
            FFileHelper::LoadFileToString(Text, *FilePath) && Text != LastFileContent)
        {
            FString Error;
            if (ApplyJson(Text, Current.StreamId.IsEmpty(), Error))
                LastFileContent = Text;
        }
    }
    for (auto I = Playing.CreateIterator(); I; ++I)
    {
        auto *A = ActorFor(I.Key());
        if (!A || (StateFor(A) && StateFor(A)->LifeStatus == TEXT("dead")))
        {
            I.RemoveCurrent();
            continue;
        }
        I.Value() += Dt;
        if (I.Value() > 2.8f)
            A->Actions->StopMeditationPresentation();
        if (I.Value() > 4.2f)
        {
            A->Actions->ResetToLocomotionPresentation();
            I.RemoveCurrent();
        }
    }
    for (int32 I = 0; I < Pending.Num();)
    {
        auto C = Pending[I];
        auto *A = ActorFor(C.ActorId);
        if (!A || (StateFor(A) && StateFor(A)->LifeStatus == TEXT("dead")))
        {
            Pending.RemoveAt(I);
            continue;
        }
        const auto *State = StateFor(A);
        if (State && State->LifeStatus == TEXT("dead"))
        {
            Pending.RemoveAt(I);
            continue;
        }
        if (Playing.Contains(C.ActorId))
        {
            ++I;
            continue;
        }
        A->Actions->ResetToLocomotionPresentation();
        A->Actions->StartMeditationPresentation();
        Playing.Add(C.ActorId, 0);
        ++PlayedPreviewCount;
        UE_LOG(LogTemp, Display, TEXT("SNAPSHOT preview EventId=%s actor=%s cosmetic only"),
               *C.EventId, *C.ActorId);
        Pending.RemoveAt(I);
    }
}
