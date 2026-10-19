#include "SnapshotPresentationAdapter.h"
#include "AppearanceComponent.h"
#include "AuraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GateCharacter.h"
#include "GateGameMode.h"
#include "Engine/World.h"
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
void USnapshotPresentationAdapter::SetPlayerActor(AGateCharacter *Player)
{
    PlayerActor = Player;
    // Warm the bounded, existing proxy visual family once per session. Refreshes and
    // new identities then use resident assets rather than per-NPC disk streaming.
    for (const TCHAR *Path : {TEXT("/Game/DaoVoTan/Data/DA_Player"),
                              TEXT("/Game/DaoVoTan/Data/DA_NPC_A"),
                              TEXT("/Game/DaoVoTan/Data/DA_NPC_B")})
        BaseRecipes.Add(LoadObject<UPresentationRecipe>(nullptr, Path));
    auto Warm = [this](const FString &Path) {
        if (auto *Asset = LoadObject<UObject>(nullptr, *Path)) CachedVisualAssets.Add(Asset);
    };
    for (const TCHAR *Name : {TEXT("M_Robe"), TEXT("M_Embroidery"), TEXT("M_Skin"),
                             TEXT("M_Hair"), TEXT("SM_Jian"), TEXT("AS_SwordIdle"),
                             TEXT("AS_SwordAttackA")})
        Warm(FString(TEXT("/Game/DaoVoTan/Visual11/")) + Name);
    for (int32 I=0; I<3; ++I) {
        Warm(FString::Printf(TEXT("/Game/DaoVoTan/Visual11/SK_Robe%d"), I));
        Warm(FString::Printf(TEXT("/Game/DaoVoTan/Visual11/SM_Head%d"), I));
    }
    for (int32 I=0; I<4; ++I) Warm(FString::Printf(TEXT("/Game/DaoVoTan/Visual11/SM_Hair%d"), I));
    for (int32 I=0; I<2; ++I) Warm(FString::Printf(TEXT("/Game/DaoVoTan/Visual11/SM_Accessory%d"), I));
    FParse::Value(FCommandLine::Get(), TEXT("SnapshotFile="), FilePath);
    if (!FilePath.IsEmpty()) LoadSnapshotFile(FilePath, true);
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
UPresentationRecipe *USnapshotPresentationAdapter::RecipeForSeed(const FString &Seed)
{
    const uint32 Token = uint32(FCString::Strtoui64(*Seed.Left(8), nullptr, 16));
    auto *Recipe = DuplicateObject<UPresentationRecipe>(BaseRecipes[Token % BaseRecipes.Num()], this);
    Recipe->RealmLabel = FText::FromString(TEXT("Presentation Alias"));
    Recipe->ApplyAppearanceSeed(Seed);
    return Recipe;
}
void USnapshotPresentationAdapter::ApplyAppearance(AGateCharacter *Actor, const FString &Seed)
{
    auto *Recipe = RecipeForSeed(Seed);
    Actor->Recipe = Recipe;
    Actor->Appearance->Apply(Recipe);
    Actor->Aura->Configure(Recipe->Aura);
    Actor->Blade->SetVisibility(Recipe->bSword);
    Actor->Guard->SetVisibility(false);
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
    if (!IsValid(PlayerActor)) {
        auto *PC = GetWorld()->GetFirstPlayerController();
        PlayerActor = PC ? Cast<AGateCharacter>(PC->GetPawn()) : nullptr;
    }
    if (!Next.ControlledActorId.IsEmpty() && !IsValid(PlayerActor))
        return Reject(TEXT("Controlled Agent requires the existing player pawn"));
    if (BaseRecipes.Num()!=3 || BaseRecipes.Contains(nullptr))
        return Reject(TEXT("Proxy presentation recipes not initialized"));
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
    // All input is valid. Build an ID-keyed plan, preserving Actors for retained IDs.
    TMap<FString, TObjectPtr<AGateCharacter>> Plan;
    TSet<AGateCharacter *> Used;
    TArray<AGateCharacter *> Spawned;
    if (!Next.ControlledActorId.IsEmpty()) {
        Plan.Add(Next.ControlledActorId, PlayerActor); Used.Add(PlayerActor);
    }
    for (const auto &State : Next.Actors) {
        if (State.ActorId==Next.ControlledActorId) continue;
        auto *Old=ActorFor(State.ActorId);
        if (IsValid(Old) && Old!=PlayerActor && !Used.Contains(Old)) {
            Plan.Add(State.ActorId, Old); Used.Add(Old);
        }
    }
    for (const auto &State : Next.Actors) {
        if (Plan.Contains(State.ActorId)) continue;
        FVector Position;
        // These are local display anchors, never identity or logical world location.
        for (int32 Cell=0; Cell<10; ++Cell) {
            Position=FVector(-11600+(Cell/3)*260, (Cell%3-1)*330, 110);
            bool Occupied=false;
            for (const auto &Entry : Plan)
                if (Entry.Value!=PlayerActor && FVector::DistSquared2D(Entry.Value->GetActorLocation(),Position)<10000)
                    Occupied=true;
            if (!Occupied) break;
        }
        FTransform Transform(FRotator(0,180,0),Position);
        auto *Proxy=GetWorld()->SpawnActorDeferred<AGateCharacter>(AGateCharacter::StaticClass(),
            Transform,GetOwner(),nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
        if (!Proxy) {
            for (auto *Created : Spawned) Created->Destroy();
            return Reject(TEXT("Could not spawn presentation proxy"));
        }
        Proxy->Recipe=RecipeForSeed(State.AppearanceSeed);
        Proxy->SetActorHiddenInGame(true); Proxy->SetActorEnableCollision(false);
        Proxy->FinishSpawning(Transform);
        Spawned.Add(Proxy); Plan.Add(State.ActorId,Proxy); Used.Add(Proxy);
    }
    if (Current.StreamId.IsEmpty())
        if (auto *Mode=Cast<AGateGameMode>(GetOwner())) Mode->RetireOfflinePreviewActors();
    SpawnedProxyCount+=Spawned.Num();
    if (NewStream) {
        if (!Current.StreamId.IsEmpty()) RetiredStreams.Add(Current.StreamId);
        Pending.Empty(); Playing.Empty();
        for (const auto &Entry : Bindings)
            if (IsValid(Entry.Value)) Entry.Value->Actions->ResetToLocomotionPresentation();
    }
    for (const auto &Entry : Bindings) {
        auto *Old=Entry.Value.Get();
        if (!IsValid(Old) || Used.Contains(Old)) continue;
        Old->Actions->ResetToLocomotionPresentation();
        Old->SetActorHiddenInGame(true); Old->SetActorEnableCollision(false);
        if (Old!=PlayerActor) { Old->Destroy(); ++RetiredProxyCount; }
    }
    if (IsValid(PlayerActor) && !Used.Contains(PlayerActor)) {
        PlayerActor->Actions->ResetToLocomotionPresentation();
        PlayerActor->SetActorHiddenInGame(true); PlayerActor->SetActorEnableCollision(false);
    }
    for (const auto &State : Next.Actors) {
        auto *Actor=Plan[State.ActorId].Get();
        const auto *Old=StateFor(Actor);
        if (!Spawned.Contains(Actor) && (!Old || Old->ActorId!=State.ActorId || Old->AppearanceSeed!=State.AppearanceSeed))
            ApplyAppearance(Actor,State.AppearanceSeed);
        Actor->SetActorHiddenInGame(false); Actor->SetActorEnableCollision(true);
        Actor->Nameplate->SetText(FText::FromString(
            FString(Actor==PlayerActor?TEXT("Player\n"):TEXT("NPC\n"))+State.ActorId));
        if (Old && Old->LifeStatus==TEXT("dead") && State.LifeStatus!=TEXT("dead"))
            Actor->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        if (State.LifeStatus==TEXT("dead")) {
            Actor->Actions->ResetToLocomotionPresentation();
            Actor->GetCharacterMovement()->DisableMovement();
        }
    }
    Pending.RemoveAll([&](const FSnapshotCue &Cue){ return !Plan.Contains(Cue.ActorId); });
    for (auto I=Playing.CreateIterator();I;++I) if (!Plan.Contains(I.Key())) I.RemoveCurrent();
    for (auto I=CommandPreviewActors.CreateIterator();I;++I) if (!Plan.Contains(*I)) I.RemoveCurrent();
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
        if (!A->Actions->IsMeditating())
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
        // A historical projection cue is already completed. Its cosmetic lifecycle
        // shares the same centralized minimum; it does not create a command.
        A->Actions->BeginCommandMeditation(1);
        A->Actions->ResolveCommandMeditation(true);
        Playing.Add(C.ActorId, 0);
        ++PlayedPreviewCount;
        UE_LOG(LogTemp, Display, TEXT("SNAPSHOT preview EventId=%s actor=%s cosmetic only"),
               *C.EventId, *C.ActorId);
        Pending.RemoveAt(I);
    }
}
