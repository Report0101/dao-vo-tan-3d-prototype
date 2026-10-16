#include "GateHUD.h"
#include "SnapshotPresentationAdapter.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GateCharacter.h"
#include "PresentationActions.h"
#include "PresentationRecipe.h"
void AGateHUD::DrawHUD()
{
    Super::DrawHUD();
    auto *C = Cast<AGateCharacter>(GetOwningPawn());
    if (!C || !Canvas)
        return;
    const float W = Canvas->SizeX, H = Canvas->SizeY;
    auto *Mode = GetWorld()->GetAuthGameMode();
    auto *Adapter = Mode ? Mode->FindComponentByClass<USnapshotPresentationAdapter>() : nullptr;
    const auto *State = Adapter ? Adapter->StateFor(C) : nullptr;
    DrawRect(FLinearColor(.015f, .025f, .03f, .85f), 28, 28, 570, State ? 160 : 117);
    DrawRect(FLinearColor(.7f, .53f, .26f, 1), 28, 28, 3, 117);
    DrawText(TEXT("DAO VO TAN  /  STILLWATER VALLEY"), FLinearColor(.85f, .72f, .45f), 47, 42,
             GEngine->GetSmallFont(), 1.1f);
    if (State)
    {
        DrawText(State->DisplayName, FLinearColor(.94f, .94f, .87f), 47, 68, GEngine->GetMediumFont(), 1.1f);
        const FString Cultivation =
            State->bHasCultivation
                ? FString::Printf(TEXT("%s / Completed sessions: %lld"),
                                  State->bHasMethod ? TEXT("Method adopted") : TEXT("No method"),
                                  State->CompletedSessions)
                : TEXT("Cultivation: unknown");
        DrawText(Cultivation, FLinearColor(.6f, .9f, .75f), 47, 99, GEngine->GetSmallFont(), 1.1f);
        DrawText(TEXT("Life: ") + State->LifeStatus + TEXT(" / Region: ") + State->RegionId,
                 FLinearColor(.8f, .85f, .8f), 47, 125, GEngine->GetSmallFont());
        DrawText(FString::Printf(TEXT("Committed tick: %lld days / aliases and XYZ are presentation only"),
                                 Adapter->Snapshot().WorldTick),
                 FLinearColor(.7f, .8f, .75f), 47, 150, GEngine->GetSmallFont());
    }
    else
    {
        DrawText(C->Recipe ? C->Recipe->DisplayName.ToString() : TEXT("Wanderer"),
                 FLinearColor(.94f, .94f, .87f), 47, 67, GEngine->GetLargeFont(), 1.1f);
        DrawText(TEXT("Presentation Fixture / ") +
                     (C->Recipe ? C->Recipe->RealmLabel.ToString() : TEXT("No realm")),
                 FLinearColor(.6f, .76f, .72f), 47, 109, GEngine->GetSmallFont());
    }
    DrawRect(FLinearColor(.01f, .02f, .025f, .8f), 28, H - 86, W - 56, 58);
    DrawText(TEXT("WASD  Move    Shift  Walk/Run    Space  Jump    LMB  Sword    E / M  Meditate    1  Aura "
                  "preset"),
             FLinearColor(.86f, .86f, .8f), 46, H - 72, GEngine->GetSmallFont(), 1.05f);
    DrawText(State ? TEXT("READ-ONLY SIM SNAPSHOT / local controls are cosmetic; no commands sent")
                   : TEXT("OFFLINE PRESENTATION FIXTURE / No simulation outcomes"),
             FLinearColor(.55f, .68f, .66f), 46, H - 49, GEngine->GetSmallFont());
    if (C->Actions->IsMeditating())
    {
        FString Label =
            C->Actions->GetPhase() == EPresentationPhase::MeditationLoop
                ? (State ? TEXT("COSMETIC MEDITATION PREVIEW") : TEXT("CULTIVATING...   [E] Rise"))
                : TEXT("Settling / rising...");
        DrawText(Label, FLinearColor(.5f, 1, .8f), W * .5f - 130, H * .72f, GEngine->GetMediumFont(), 1.2f);
    }
}
