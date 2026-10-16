#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "CultivationCommandAdapter.h"
#include "CultivationReceipt.h"
#include "SnapshotPresentationAdapter.h"
#include "GateCharacter.h"
#include "PresentationActions.h"
#include "AuraComponent.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UnrealClient.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCultivationRoundTrip, "DaoVoTan.Integration.CommandRoundTripPIE",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
class FCommandExercise : public IAutomationLatentCommand
{
    FAutomationTestBase *Test;
    int32 Stage = 0;
    double Started = FPlatformTime::Seconds(), Since = Started;
    FString PositiveRequest, PositiveAction, PositiveEvent;
    int64 BeforeTick = 0, AfterTick = 0;
    bool Enabled = false;
    double ParseMs = 0, UpdateMs = 0, SerializeMs = 0;
    UWorld *World()
    {
        for (const auto &C : GEngine->GetWorldContexts())
            if (C.WorldType == EWorldType::PIE)
                return C.World();
        return nullptr;
    }
    FString Dir()
    {
        return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Gate2Evidence"));
    }
    void Shot(const TCHAR *Name)
    {
        FScreenshotRequest::RequestScreenshot(FPaths::Combine(Dir(), Name), false, false, false, FIntRect(),
                                              true);
    }
    void Next()
    {
        ++Stage;
        Since = FPlatformTime::Seconds();
    }

  public:
    explicit FCommandExercise(FAutomationTestBase *T) : Test(T) {}
    virtual bool Update() override
    {
        const double Now = FPlatformTime::Seconds();
        if (Now - Started > 90)
        {
            Test->AddError(TEXT("Gate2 real worker/PIE round trip timed out"));
            return true;
        }
        auto *W = World();
        auto *PC = W ? W->GetFirstPlayerController() : nullptr;
        if (!PC || !W->GetAuthGameMode())
            return false;
        auto *C = PC->FindComponentByClass<UCultivationCommandAdapter>();
        auto *A = W->GetAuthGameMode()->FindComponentByClass<USnapshotPresentationAdapter>();
        if (!C || !A)
            return false;
        if (!Enabled)
        {
            C->SetTransportEnabled(true);
            Enabled = true;
        }
        const double T = Now - Since;
        switch (Stage)
        {
        case 0:
        {
            if (A->Snapshot().WorldId != TEXT("command1") || A->BindingCount() != 3 ||
                !C->CultivateInputAction())
                return false;
            BeforeTick = A->Snapshot().WorldTick;
            C->SelectedDays = 7;
            auto *Input =
                ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
            if (!Input)
            {
                Test->AddError(TEXT("Missing Enhanced Input subsystem"));
                return true;
            }
            Input->InjectInputForAction(C->CultivateInputAction(), FInputActionValue(true), {}, {});
            Next();
            break;
        }
        case 1:
            if (C->LastRequestId.IsEmpty())
                return false;
            PositiveRequest = C->LastRequestId;
            Test->TestTrue(TEXT("Same Unreal request resent before completion"), C->ResendLastRequest());
            Test->TestTrue(TEXT("Duplicate transport send succeeds"), C->ResendLastRequest());
            Next();
            break;
        case 2:
            if (C->Receipt.Status != TEXT("completed") || A->Snapshot().WorldTick != C->Receipt.ResolvedTick)
                return false;
            AfterTick = A->Snapshot().WorldTick;
            ParseMs = C->LastParseMs;
            UpdateMs = C->LastUpdateMs;
            SerializeMs = C->LastSerializeMs;
            PositiveAction = C->Receipt.ActionId;
            PositiveEvent = C->Receipt.SessionEventId;
            Test->TestEqual(TEXT("One strategic time cost using existing requested days"),
                            AfterTick - BeforeTick, int64(7));
            Test->TestEqual(TEXT("One logical completion after duplicate sends"), C->LogicalResultCount, 1);
            Test->TestEqual(TEXT("Exactly one cosmetic sequence"), C->PreviewStartCount, 1);
            Test->TestEqual(TEXT("Snapshot did not replay command completion separately"),
                            A->PlayedPreviewCount, 0);
            {
                auto *Actor = A->ActorFor(A->Snapshot().ControlledActorId);
                const auto *State = Actor ? A->StateFor(Actor) : nullptr;
                Test->TestTrue(TEXT("Real committed cultivation count reaches UI"),
                               State && State->CompletedSessions == 1 && State->bHasMethod);
            }
            Next();
            break;
        case 3:
            if (T < 1.5)
                return false;
            {
                auto *Actor = A->ActorFor(A->Snapshot().ControlledActorId);
                Test->TestTrue(TEXT("Existing meditation/aura presentation shows real command"),
                               Actor && Actor->Actions->IsMeditating() && Actor->Aura->IsEmitting());
            }
            Shot(TEXT("01-authoritative-cultivation.png"));
            {
                const FString Good = C->LastRequestId;
                const int32 Results = C->LogicalResultCount;
                FString Error;
                Test->TestFalse(TEXT("Malformed receipt rejected atomically"),
                                C->ConsumeReceipt(TEXT("{\"schemaVersion\":2}"), Error));
                Test->TestEqual(TEXT("Malformed receipt changes no logical result"), C->LogicalResultCount,
                                Results);
                Test->TestEqual(TEXT("Request identity unchanged"), C->LastRequestId, Good);
            }
            Test->TestTrue(TEXT("Resend completed request"), C->ResendLastRequest());
            Next();
            break;
        case 4:
            if (T < 4)
                return false;
            Test->TestEqual(TEXT("No repeated result"), C->LogicalResultCount, 1);
            Test->TestEqual(TEXT("No second preview"), C->PreviewStartCount, 1);
            Test->TestEqual(TEXT("No duplicate strategic advance"), A->Snapshot().WorldTick, AfterTick);
            {
                auto *Actor = A->ActorFor(A->Snapshot().ControlledActorId);
                Test->TestTrue(TEXT("Returns to idle after local cosmetic time"),
                               Actor && !Actor->Actions->IsMeditating() && !Actor->Aura->IsEmitting());
            }
            Shot(TEXT("02-completed-idle.png"));
            Next();
            break;
        case 5:
            if (T < 1)
                return false;
            Test->TestTrue(TEXT("Send one intentionally invalid actor for authoritative rejection"),
                           C->SendCultivateForActor(TEXT("command1:agent:99999999"), 7));
            Next();
            break;
        case 6:
            if (C->Receipt.Status != TEXT("rejected"))
                return false;
            Test->TestEqual(TEXT("Simulation owns typed rejection"), C->Receipt.RejectionCode,
                            FString(TEXT("WRONG_ACTOR")));
            Test->TestEqual(TEXT("Rejection starts no meditation"), C->PreviewStartCount, 1);
            Test->TestEqual(TEXT("Rejection advances no strategic time"), A->Snapshot().WorldTick, AfterTick);
            Shot(TEXT("03-authoritative-rejection.png"));
            Next();
            break;
        default:
            if (T < 1)
                return false;
            {
                const FString Summary = FString::Printf(
                    TEXT("request_id=%s\naction_instance_id=%s\ncultivation_event_id=%s\ntick_before=%"
                         "lld\ntick_after=%lld\nstrategic_days=7\ncosmetic_sequences=%d\ncompletion_results_"
                         "before_negative_case=1\nrejection_code=%s\nreceipt_parse_ms=%.6f\nreceipt_update_"
                         "ms=%.6f\ncommand_serialize_ms=%.6f\n"),
                    *PositiveRequest, *PositiveAction, *PositiveEvent, BeforeTick, AfterTick,
                    C->PreviewStartCount, *C->Receipt.RejectionCode, ParseMs, UpdateMs, SerializeMs);
                FFileHelper::SaveStringToFile(Summary, *FPaths::Combine(Dir(), TEXT("round-trip-proof.txt")));
                Test->AddInfo(Summary);
            }
            return true;
        }
        return false;
    }
};
bool FCultivationRoundTrip::RunTest(const FString &Params)
{
    if (!FParse::Param(FCommandLine::Get(), TEXT("CultivationGate2Test")))
    {
        AddInfo(TEXT(
            "Live worker fixture requires -CultivationGate2Test -CultivationBridgeDir=<prepared directory>"));
        return true;
    }
    ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/DaoVoTan/Maps/StillwaterValley")));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitForShadersToFinishCompiling());
    ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
    ADD_LATENT_AUTOMATION_COMMAND(FCommandExercise(this));
    ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCultivationReceiptValidation, "DaoVoTan.Integration.CommandReceiptProtocol",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCultivationReceiptValidation::RunTest(const FString &Params)
{
    const FString Valid =
        TEXT("{\"schemaVersion\":1,\"worldId\":\"command1\",\"requestId\":\"fixture\",\"actorId\":\"command1:"
             "agent:00000001\",\"status\":\"completed\",\"rejectionCode\":null,\"actionInstanceId\":"
             "\"command1:action:00000001\",\"acceptedEventId\":\"command1:event:00000001\","
             "\"resultEventIds\":[\"command1:event:00000002\"],\"startTick\":5,\"dueTick\":12,"
             "\"resolvedTick\":12,\"cultivationSessionEventId\":\"command1:event:00000002\"}");
    FCultivationReceipt R;
    FString Error;
    TestTrue(TEXT("Valid completed receipt"), CultivationProtocol::ParseReceipt(Valid, R, Error));
    const FString Original = R.RequestId;
    for (const auto &Bad :
         {Valid.Replace(TEXT("\"schemaVersion\":1"), TEXT("\"schemaVersion\":2")),
          Valid.Replace(TEXT("\"resolvedTick\":12"), TEXT("\"resolvedTick\":11")),
          Valid.Replace(TEXT("\"completed\""), TEXT("\"magic\"")),
          Valid.Replace(TEXT("\"dueTick\":12"), TEXT("\"dueTick\":9007199254740992")), FString(TEXT("{"))})
    {
        TestFalse(TEXT("Bad receipt rejected"), CultivationProtocol::ParseReceipt(Bad, R, Error));
        TestEqual(TEXT("No partial parsed output"), R.RequestId, Original);
    }
    return true;
}
#endif
