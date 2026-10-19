#include "GateGameMode.h"
#include "SnapshotPresentationAdapter.h"
#include "TimerManager.h"
#include "GateCharacter.h"
#include "GateHUD.h"
#include "GatePlayerController.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "PresentationRecipe.h"
#include "PresentationActions.h"
AGateGameMode::AGateGameMode()
{
    SnapshotAdapter = CreateDefaultSubobject<USnapshotPresentationAdapter>(TEXT("SnapshotAdapter"));
    DefaultPawnClass = AGateCharacter::StaticClass();
    PlayerControllerClass = AGatePlayerController::StaticClass();
    HUDClass = AGateHUD::StaticClass();
}
void AGateGameMode::BeginPlay()
{
    Super::BeginPlay();
    int32 Count = 2;
    FParse::Value(FCommandLine::Get(), TEXT("GateNPCs="), Count);
    Count = FMath::Clamp(Count, 2, 10);
    // Offline preview actors are retired when the first valid snapshot arrives.
    const bool Integration = FParse::Param(FCommandLine::Get(), TEXT("SnapshotIntegrationTest")) ||
                             FParse::Param(FCommandLine::Get(), TEXT("SnapshotIntegration")) ||
                             (FString(FCommandLine::Get()).Contains(TEXT("SnapshotFile=")) ||
                              FString(FCommandLine::Get()).Contains(TEXT("CultivationBridgeDir=")));
    for (int32 I = 0; !Integration && I < Count; ++I)
    {
        auto *R = LoadObject<UPresentationRecipe>(nullptr, I % 2 ? TEXT("/Game/DaoVoTan/Data/DA_NPC_B")
                                                                 : TEXT("/Game/DaoVoTan/Data/DA_NPC_A"));
        FVector Location(-11800 + (I / 2) * 180, I % 2 ? 280 : -280, 8);
        FTransform T(FRotator(0, I % 2 ? -90 : 90, 0), Location);
        auto *NPC = GetWorld()->SpawnActorDeferred<AGateCharacter>(
            AGateCharacter::StaticClass(), T, nullptr, nullptr,
            ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
        NPC->Recipe = R;
        NPC->FinishSpawning(T);
        OfflinePreviewActors.Add(NPC);
        // Explicit fixture pose, not an NPC decision or cultivation outcome.
        if (!Integration && I % 2 == 0)
            NPC->Actions->StartMeditationPresentation();
    }
    GetWorldTimerManager().SetTimerForNextTick(
        [this]()
        {
            auto *PC = GetWorld()->GetFirstPlayerController();
            SnapshotAdapter->SetPlayerActor(PC ? Cast<AGateCharacter>(PC->GetPawn()) : nullptr);
        });
    UE_LOG(LogTemp, Display, TEXT("GATE1 offline fixture: player + %d NPCs. No simulation authority."),
           Count);
}

void AGateGameMode::RetireOfflinePreviewActors()
{
    for (auto Actor : OfflinePreviewActors) if (IsValid(Actor)) Actor->Destroy();
    OfflinePreviewActors.Empty();
}
