#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "Engine/Engine.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "CultivationCommandAdapter.h"
#include "SnapshotPresentationAdapter.h"
#include "GateCharacter.h"
#include "PresentationActions.h"
#include "InputKeyEventArgs.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/FileHelper.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "PresentationEvidence.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCultivationReconnect, "DaoVoTan.Cultivation.StabilizationPIE",
                                 EAutomationTestFlags::EditorContext |
                                     EAutomationTestFlags::EngineFilter)
struct FReconnectProof
{
    TSet<FString> Sessions;
    int64 Tick = -1;
    FString Trace;
};
class FStabilizationExercise : public IAutomationLatentCommand
{
    FAutomationTestBase *Test;
    TSharedRef<FReconnectProof> Proof;
    int32 Session, Stage = 0, Command = 0;
    double Started = FPlatformTime::Seconds(), Since = 0;
    int64 Before = 0;
    int32 CompletionBefore = 0;
    bool SawEarlyCompletion = false;
    void Raw(APlayerController *PC, FKey Key, EInputEvent Event)
    {
        PC->InputKey(
            FInputKeyEventArgs::CreateSimulated(Key, Event, Event == IE_Pressed ? 1.f : 0.f));
    }

  public:
    FStabilizationExercise(FAutomationTestBase *T, TSharedRef<FReconnectProof> P, int32 S)
        : Test(T), Proof(P), Session(S)
    {
    }
    bool Update() override
    {
        if (FPlatformTime::Seconds() - Started > 90)
        {
            Test->AddError(TEXT("Reconnect fixture timed out"));
            return true;
        }
        UWorld *W = nullptr;
        for (const auto &C : GEngine->GetWorldContexts())
            if (C.WorldType == EWorldType::PIE)
                W = C.World();
        auto *PC = W ? W->GetFirstPlayerController() : nullptr;
        auto *A = W && W->GetAuthGameMode()
                      ? W->GetAuthGameMode()->FindComponentByClass<USnapshotPresentationAdapter>()
                      : nullptr;
        auto *C = PC ? PC->FindComponentByClass<UCultivationCommandAdapter>() : nullptr;
        auto *Player = PC ? Cast<AGateCharacter>(PC->GetPawn()) : nullptr;
        if (!A || !C || !Player)
            return false;
        auto *Actions = Player->Actions.Get();
        const int32 Days = Session == 0 ? 1 : Session == 1 ? 7 : Command == 0 ? 17 : 30;
        if (Stage == 0)
        {
            // EndPIE/StartPIE are asynchronous. Never exercise the retiring world's adapter.
            if (Proof->Sessions.Contains(C->PresentationSessionId))
                return false;
            if (A->Snapshot().StreamId != C->PresentationSessionId ||
                A->Snapshot().ControlledActorId.IsEmpty())
                return false;
            Test->TestFalse(TEXT("New adapter session identity"),
                            Proof->Sessions.Contains(C->PresentationSessionId));
            Proof->Sessions.Add(C->PresentationSessionId);
            Test->TestEqual(TEXT("Full resync begins sequence 1"), A->Snapshot().Sequence,
                            int64(1));
            Test->TestEqual(TEXT("No historical cues in resync"), A->Snapshot().Cues.Num(), 0);
            Test->TestEqual(TEXT("No old receipt presentation"), C->PreviewStartCount, 0);
            Test->TestEqual(TEXT("Controlled actor bound to possessed pawn"),
                            A->ActorFor(A->Snapshot().ControlledActorId), Player);
            if (Proof->Tick >= 0)
                Test->TestEqual(TEXT("Persistent world survives PIE stop"), A->Snapshot().WorldTick,
                                Proof->Tick);
            for (int32 D = 1; D <= 30; ++D)
            {
                C->SelectCultivationDays(D);
                Test->TestEqual(TEXT("Every integer selectable"), C->GetSelectedCultivationDays(),
                                D);
            }
            C->SelectCultivationDays(0);
            Test->TestEqual(TEXT("Clamp 0"), C->GetSelectedCultivationDays(), 1);
            C->SelectCultivationDays(31);
            Test->TestEqual(TEXT("Clamp 31"), C->GetSelectedCultivationDays(), 30);
            Raw(PC, EKeys::Hyphen, IE_Pressed);
            Since = W->GetTimeSeconds();
            Stage = 4;
            return false;
        }
        if (Stage == 4)
        {
            Raw(PC, EKeys::Hyphen, IE_Released);
            if (W->GetTimeSeconds() - Since < .1)
                return false;
            Test->TestEqual(TEXT("Minus raw key decreases by one"), C->GetSelectedCultivationDays(),
                            29);
            Raw(PC, EKeys::Equals, IE_Pressed);
            Since = W->GetTimeSeconds();
            Stage = 5;
            return false;
        }
        if (Stage == 5)
        {
            Raw(PC, EKeys::Equals, IE_Released);
            if (W->GetTimeSeconds() - Since < .1)
                return false;
            Test->TestEqual(TEXT("Plus raw key increases by one"), C->GetSelectedCultivationDays(),
                            30);
            Stage = 1;
        }
        if (Stage == 1)
        {
            C->SelectCultivationDays(Days);
            Before = A->Snapshot().WorldTick;
            CompletionBefore = Actions->MeditationCompletionCount;
            Raw(PC, Session == 1 ? EKeys::M : EKeys::E, IE_Pressed);
            Since = W->GetTimeSeconds();
            Stage = 2;
            return false;
        }
        if (Stage == 2)
        {
            Raw(PC, Session == 1 ? EKeys::M : EKeys::E, IE_Released);
            if (C->Receipt.Status != TEXT("completed"))
                return false;
            TSharedPtr<FJsonObject> Request;
            Test->TestTrue(TEXT("Request JSON"),
                           FJsonSerializer::Deserialize(
                               TJsonReaderFactory<>::Create(C->LastRequestJson), Request));
            if (Request)
                Test->TestEqual(
                    TEXT("Selected days unchanged in command"),
                    int32(Request->GetObjectField(TEXT("command"))->GetNumberField(TEXT("days"))),
                    Days);
            Test->TestEqual(TEXT("Authoritative strategic duration"),
                            C->Receipt.DueTick - C->Receipt.StartTick, int64(Days));
            Test->TestEqual(TEXT("World advanced only selected days"), C->Receipt.ResolvedTick,
                            Before + Days);
            if (Actions->CommandVisualAge() < UPresentationActions::CultivationVisualSeconds(Days))
            {
                SawEarlyCompletion = true;
                Test->TestTrue(TEXT("Early result retained while still meditating"),
                               Actions->IsMeditating());
                Test->TestEqual(TEXT("No premature visual completion"),
                                Actions->MeditationCompletionCount, CompletionBefore);
                CapturePIEEvidence(
                    FPaths::ProjectSavedDir() /
                    FString::Printf(TEXT("StabilizationEvidence/session-%d-days-%d-meditating.png"),
                                    Session + 1, Days));
            }
            FString Error;
            FString ReceiptJson;
            const FString Bridge = [&]
            {
                FString B;
                FParse::Value(FCommandLine::Get(), TEXT("CultivationBridgeDir="), B);
                return B;
            }();
            FFileHelper::LoadFileToString(
                ReceiptJson, *(Bridge / TEXT("outbox") / (C->LastRequestId + TEXT(".json"))));
            Test->TestTrue(TEXT("Duplicate receipt accepted idempotently"),
                           C->ConsumeReceipt(ReceiptJson, Error));
            Test->TestEqual(TEXT("Duplicate does not restart preview"), C->PreviewStartCount,
                            Command + 1);
            Proof->Trace +=
                FString::Printf(TEXT("session=%d id=%s days=%d request=%s action=%s start=%lld "
                                     "resolved=%lld early=%d\n"),
                                Session + 1, *C->PresentationSessionId, Days, *C->LastRequestId,
                                *C->Receipt.ActionId, C->Receipt.StartTick, C->Receipt.ResolvedTick,
                                SawEarlyCompletion);
            Stage = 3;
        }
        if (Stage == 3)
        {
            if (Actions->IsMeditating())
                return false;
            Test->TestTrue(TEXT("Readable minimum before exit"),
                           Actions->CommandVisualAge() >=
                               UPresentationActions::CultivationVisualSeconds(Days));
            Test->TestEqual(TEXT("One cosmetic completion"), Actions->MeditationCompletionCount,
                            CompletionBefore + 1);
            Test->TestEqual(TEXT("One result per input"), C->LogicalResultCount, Command + 1);
            Test->TestTrue(TEXT("Fast authoritative completion observed before visual minimum"),
                           SawEarlyCompletion);
            Proof->Trace += FString::Printf(
                TEXT("visual_recovered session=%d days=%d age=%.3f minimum=%.3f completions=%d\n"),
                Session + 1, Days, Actions->CommandVisualAge(),
                UPresentationActions::CultivationVisualSeconds(Days),
                Actions->MeditationCompletionCount);
            Proof->Tick = C->Receipt.ResolvedTick;
            CapturePIEEvidence(FPaths::ProjectSavedDir() /
                               FString::Printf(TEXT("StabilizationEvidence/session-%d-days-%d.png"),
                                               Session + 1, Days));
            if (Session == 2 && Command == 0)
            {
                ++Command;
                Stage = 1;
                return false;
            }
            // Controlled clock proof: expiration without a receipt must never fake completion.
            Actions->BeginCommandMeditation(30);
            Actions->TickComponent(1.3f, LEVELTICK_All, nullptr);
            Actions->TickComponent(7.f, LEVELTICK_All, nullptr);
            Test->TestEqual(TEXT("Late result stays in loop after minimum"), Actions->GetPhase(),
                            EPresentationPhase::MeditationLoop);
            Actions->ResolveCommandMeditation(true);
            Actions->TickComponent(.01f, LEVELTICK_All, nullptr);
            Test->TestEqual(TEXT("Late result releases exit"), Actions->GetPhase(),
                            EPresentationPhase::MeditationExit);
            Actions->TickComponent(1.3f, LEVELTICK_All, nullptr);
            Test->TestEqual(TEXT("Late lifecycle returns idle"), Actions->GetPhase(),
                            EPresentationPhase::Locomotion);
            FFileHelper::SaveStringToFile(
                Proof->Trace, *(FPaths::ProjectSavedDir() /
                                TEXT("StabilizationEvidence/reconnect-duration-proof.txt")));
            return true;
        }
        return false;
    }
};
bool FCultivationReconnect::RunTest(const FString &)
{
    if (!FParse::Param(FCommandLine::Get(), TEXT("StabilizationTest")))
    {
        AddInfo(TEXT("Dedicated persistent worker fixture required"));
        return true;
    }
    auto Proof = MakeShared<FReconnectProof>();
    ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/DaoVoTan/Maps/StillwaterValley")));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitForShadersToFinishCompiling());
    for (int32 S = 0; S < 3; ++S)
    {
        ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
        ADD_LATENT_AUTOMATION_COMMAND(FStabilizationExercise(this, Proof, S));
        ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
    }
    return true;
}
#endif
