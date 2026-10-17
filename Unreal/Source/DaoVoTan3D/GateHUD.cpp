#include "GateHUD.h"
#include "CultivationCommandAdapter.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/TextureRenderTarget2D.h"
#include "EngineUtils.h"
#include "GameFramework/GameModeBase.h"
#include "GateCharacter.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "PortraitComponent.h"
#include "PresentationActions.h"
#include "PresentationRecipe.h"
#include "SnapshotPresentationAdapter.h"
void AGateHUD::DrawHUD()
{
    Super::DrawHUD();
    auto *C = Cast<AGateCharacter>(GetOwningPawn());
    if (!C || !Canvas)
        return;
    const float W = Canvas->SizeX, H = Canvas->SizeY;
    auto *Mode = GetWorld()->GetAuthGameMode();
    auto *A = Mode ? Mode->FindComponentByClass<USnapshotPresentationAdapter>() : nullptr;
    const auto *S = A ? A->StateFor(C) : nullptr;
    const FLinearColor Ink(.018f, .035f, .043f, .9f), Jade(.48f, .82f, .72f),
        Gold(.79f, .65f, .37f), Text(.88f, .9f, .85f);
    auto Line = [&](const FString &T, float X, float Y, FLinearColor Color, float Scale = 1.f)
    { DrawText(T, Color, X, Y, GEngine->GetSmallFont(), Scale); };
    DrawRect(Ink, 22, 22, 500, S ? 152 : 125);
    DrawRect(Gold, 22, 22, 2, S ? 152 : 125);
    if (auto *P = C->Portrait->RequestPortrait())
        DrawTexture(P, 34, 36, 92, 92, 0, 0, 1, 1, FLinearColor::White, BLEND_Opaque);
    Line(TEXT("DAO VO TAN  /  STILLWATER"), 142, 36, Gold, 1.12f);
    Line(S ? S->DisplayName : (C->Recipe ? C->Recipe->DisplayName.ToString() : TEXT("Wanderer")),
         142, 62, Text, 1.15f);
    if (S)
    {
        Line(S->bHasCultivation
                 ? FString::Printf(TEXT("%s   /   Sessions %lld"),
                                   S->bHasMethod ? TEXT("Method adopted") : TEXT("No method"),
                                   S->CompletedSessions)
                 : TEXT("Cultivation unknown"),
             142, 91, Jade);
        Line(TEXT("Life: ") + S->LifeStatus, 142, 116, Text);
        Line(FString::Printf(TEXT("SIMULATION FACTS  /  committed day %lld"),
                             A->Snapshot().WorldTick),
             36, 151, Jade, .9f);
    }
    else
        Line(TEXT("OFFLINE VISUAL FIXTURE"), 142, 98, Jade, .95f);
    auto *Commands =
        GetOwningPlayerController()->FindComponentByClass<UCultivationCommandAdapter>();
    if (Commands)
    {
        FString Phase = TEXT("DISCONNECTED");
        if (Commands->IsConnected())
        {
            Phase = Commands->LastRequestId.IsEmpty()    ? TEXT("READY")
                    : Commands->Receipt.Status.IsEmpty() ? TEXT("REQUESTED")
                                                         : Commands->Receipt.Status.ToUpper();
            if (Commands->Receipt.Status == TEXT("accepted"))
                Phase = TEXT("ACCEPTED / RESERVED");
            if (C->Actions->IsMeditating())
                Phase += TEXT("  /  MEDITATING (visual)");
        }
        const float StatusY = H - 174;
        DrawRect(Ink, 22, StatusY, FMath::Min(420.f, W - 175), 87);
        DrawRect(Jade, 22, StatusY, 2, 87);
        Line(Phase, 36, StatusY + 11, Jade, 1.05f);
        Line(Commands->Feedback, 36, StatusY + 37, Text, .78f);
        Line(FString::Printf(TEXT("E / M  Cultivate %d strategic days    + / -  duration"),
                             Commands->SelectedDays),
             36, StatusY + 62, Gold, .78f);
    }
    int32 Index = 0;
    TArray<AGateCharacter *> PortraitActors = {C};
    for (TActorIterator<AGateCharacter> I(GetWorld()); I && PortraitActors.Num() < 3; ++I)
        if (*I != C && !I->IsHidden())
            PortraitActors.Add(*I);
    for (auto *Actor : PortraitActors)
    {
        float X = W - 124, Y = 22 + Index * 132;
        DrawRect(Ink, X, Y, 102, 122);
        DrawRect(Gold, X, Y, 102, 1);
        if (auto *P = Actor->Portrait->RequestPortrait())
            DrawTexture(P, X + 7, Y + 7, 88, 88, 0, 0, 1, 1, FLinearColor::White, BLEND_Opaque);
        Line(Index == 0   ? TEXT("PLAYER")
             : Index == 1 ? TEXT("NPC A")
                          : TEXT("NPC B"),
             X + 14, Y + 101, Jade, .9f);
        ++Index;
    }
    DrawRect(Ink, 22, H - 69, W - 44, 47);
    Line(TEXT("WASD Move   Shift Walk / Run   Q Strafe   Space Jump   LMB Sword   1 Qi palette"),
         36, H - 59, Text, .95f);
    Line(S ? TEXT("Simulation owns outcomes. Movement / sword / aura are cosmetic.")
           : TEXT("OFFLINE FIXTURE / no authoritative outcome"),
         36, H - 39, Jade, .85f);
    if (FParse::Param(FCommandLine::Get(), TEXT("GateDeveloperPreview")))
        Line(TEXT("DEVELOPER PREVIEW ONLY"), 36, H - 96, Gold);
}
