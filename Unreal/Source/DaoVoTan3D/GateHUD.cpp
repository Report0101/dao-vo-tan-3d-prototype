#include "GateHUD.h"
#include "SnapshotPresentationAdapter.h"
#include "CultivationCommandAdapter.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
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
    auto *Commands = GetOwningPlayerController()
                         ? GetOwningPlayerController()->FindComponentByClass<UCultivationCommandAdapter>()
                         : nullptr;
    if (Commands)
    {
        DrawRect(FLinearColor(.01f, .02f, .025f, .85f), 28, 202, W - 56, 62);
        DrawText(FParse::Param(FCommandLine::Get(), TEXT("GateDeveloperPreview"))
                     ? TEXT("DEVELOPER PREVIEW ONLY / [E / M] cosmetic meditation; no command")
                     : FString::Printf(TEXT("[E / M] CULTIVATE %d strategic day(s) / [+ / -] choose 1..30"),
                                       Commands->SelectedDays),
                 FLinearColor(.7f, .9f, .8f), 46, 214, GEngine->GetSmallFont());
        DrawText(Commands->Feedback, FLinearColor(.9f, .8f, .55f), 46, 238, GEngine->GetSmallFont());
    }
    DrawRect(FLinearColor(.01f, .02f, .025f, .8f), 28, H - 86, W - 56, 58);
    DrawText(TEXT("WASD  Move    Shift  Walk/Run    Space  Jump    LMB  Sword    E / M  Cultivate    1  Aura "
                  "preset"),
             FLinearColor(.86f, .86f, .8f), 46, H - 72, GEngine->GetSmallFont(), 1.05f);
    DrawText(State ? TEXT("SIMULATION OWNS CULTIVATION / movement, sword and aura presets are cosmetic")
                   : TEXT("OFFLINE PRESENTATION FIXTURE / No simulation outcomes"),
             FLinearColor(.55f, .68f, .66f), 46, H - 49, GEngine->GetSmallFont());
    if (C->Actions->IsMeditating())
    {
        FString Label =
            C->Actions->GetPhase() == EPresentationPhase::MeditationLoop
                ? (Commands && (Commands->Receipt.Status == TEXT("accepted") ||
                                Commands->Receipt.Status == TEXT("completed"))
                       ? TEXT("CULTIVATION / cosmetic playback")
                       : (State ? TEXT("COSMETIC MEDITATION PREVIEW") : TEXT("DEVELOPER MEDITATION PREVIEW")))
                : TEXT("Settling / rising...");
        DrawText(Label, FLinearColor(.5f, 1, .8f), W * .5f - 130, H * .72f, GEngine->GetMediumFont(), 1.2f);
    }
}
