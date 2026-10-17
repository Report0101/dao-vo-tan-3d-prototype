#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "GateCharacter.h"
#include "GatePlayerController.h"
#include "CultivationCommandAdapter.h"
#include "CultivatorLocomotion.h"
#include "PresentationActions.h"
#include "PresentationEvidence.h"
#include "InputKeyEventArgs.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "Components/SkeletalMeshComponent.h"
#include "Misc/FileHelper.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FManualInputRegression, "DaoVoTan.Input.ManualKeysAndRepeatPIE",
                                 EAutomationTestFlags::EditorContext |
                                     EAutomationTestFlags::EngineFilter)

class FManualInputExercise : public IAutomationLatentCommand
{
    FAutomationTestBase *Test;
    int32 Stage = 0, Attack = 0;
    double Since = 0, Started = FPlatformTime::Seconds();
    bool Released = false, SawEarly = false, SawProgress = false;
    FString Trace;
    void Next(UWorld *W)
    {
        ++Stage;
        Since = W->GetTimeSeconds();
        Released = false;
    }
    void Key(AGatePlayerController *PC, FKey K, EInputEvent Event)
    {
        // The PlayerController's real raw-key path feeds EnhancedPlayerInput,
        // mapping contexts and Started/Completed bindings; no action method call.
        PC->InputKey(
            FInputKeyEventArgs::CreateSimulated(K, Event, Event == IE_Pressed ? 1.f : 0.f));
    }
    void Shot(const TCHAR *Name)
    {
        Test->TestTrue(
            TEXT("Manual input evidence capture"),
            CapturePIEEvidence(FPaths::ProjectSavedDir() / TEXT("ManualInputEvidence") / Name));
    }

  public:
    explicit FManualInputExercise(FAutomationTestBase *T) : Test(T)
    {
    }
    bool Update() override
    {
        if (FPlatformTime::Seconds() - Started > 90)
        {
            Test->AddError(TEXT("Manual input timeout"));
            return true;
        }
        UWorld *W = nullptr;
        for (const auto &Context : GEngine->GetWorldContexts())
            if (Context.WorldType == EWorldType::PIE)
                W = Context.World();
        auto *PC = W ? Cast<AGatePlayerController>(W->GetFirstPlayerController()) : nullptr;
        auto *C = PC ? Cast<AGateCharacter>(PC->GetPawn()) : nullptr;
        auto *A = C ? Cast<UCultivatorLocomotion>(C->GetMesh()->GetAnimInstance()) : nullptr;
        if (!A)
            return false;
        auto *Commands = PC->CultivationCommands.Get();
        const double T = W->GetTimeSeconds() - Since;
        Trace +=
            FString::Printf(TEXT("time=%.6f stage=%d attack=%d phase=%d starts=%d revision=%u "
                                 "action_time=%.6f blend=%.6f input_attempts=%d status=%s\n"),
                            W->GetTimeSeconds(), Stage, Attack, int32(C->Actions->GetPhase()),
                            C->Actions->SwordStartCount, A->ActionRevision, A->ActionSequenceTime,
                            A->ActionBlendWeight, Commands->InputAttempts, *Commands->InputStatus);
        switch (Stage)
        {
        case 0:
            if (W->GetTimeSeconds() > 2)
            {
                auto *S = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
                    PC->GetLocalPlayer());
                Test->TestTrue(TEXT("Possessed pawn and command context installed"),
                               PC->GetPawn() == C &&
                                   S->HasMappingContext(Commands->CommandInputContext()));
                bool E = false, M = false;
                for (const auto &Mapping : Commands->CommandInputContext()->GetMappings())
                {
                    E |= Mapping.Key == EKeys::E;
                    M |= Mapping.Key == EKeys::M;
                }
                Test->TestTrue(TEXT("Both E and M present in actual command context"), E && M);
                Key(PC, EKeys::E, IE_Pressed);
                Next(W);
            }
            break;
        case 1:
            if (!Released)
            {
                Key(PC, EKeys::E, IE_Released);
                Released = true;
            }
            if (T > .3)
            {
                Test->TestEqual(TEXT("Raw E reaches Enhanced Input callback"),
                                Commands->InputAttempts, 1);
                Test->TestTrue(TEXT("Offline E gives visible configuration feedback"),
                               Commands->InputStatus == TEXT("CONFIG UNAVAILABLE"));
                Test->TestTrue(TEXT("No offline authoritative request fabricated"),
                               Commands->LastRequestId.IsEmpty());
                Shot(TEXT("01-E-config-unavailable.png"));
                Key(PC, EKeys::M, IE_Pressed);
                Next(W);
            }
            break;
        case 2:
            if (!Released)
            {
                Key(PC, EKeys::M, IE_Released);
                Released = true;
            }
            if (T > .3)
            {
                Test->TestEqual(TEXT("Raw M also reaches callback"), Commands->InputAttempts, 2);
                Shot(TEXT("02-M-config-unavailable.png"));
                Next(W);
            }
            break;
        case 3:
            Key(PC, EKeys::LeftMouseButton, IE_Pressed);
            SawEarly = SawProgress = false;
            Next(W);
            break;
        case 4:
            if (!Released)
            {
                Key(PC, EKeys::LeftMouseButton, IE_Released);
                Released = true;
            }
            if (C->Actions->GetPhase() == EPresentationPhase::Sword && A->ActionSequenceTime < .25f)
                SawEarly = true;
            if (C->Actions->GetPhase() == EPresentationPhase::Sword &&
                A->ActionSequenceTime > .35f && A->ActionSequenceTime < 1.2f)
                SawProgress = true;
            if (T > .4)
            {
                Key(PC, EKeys::LeftMouseButton, IE_Pressed);
                Next(W);
            }
            break;
        case 5:
            if (!Released)
            {
                Key(PC, EKeys::LeftMouseButton, IE_Released);
                Released = true;
            }
            if (A->ActionSequenceTime > .35f && A->ActionSequenceTime < 1.2f)
                SawProgress = true;
            if (T > 1.5)
            {
                Test->TestTrue(TEXT("Every click after Ready restarts sequence near zero"),
                               SawEarly);
                Test->TestTrue(TEXT("Every repeated attack advances through strike, not just enum"),
                               SawProgress);
                Test->TestEqual(TEXT("Ignore rapid click while active, no queued combo"),
                                C->Actions->SwordStartCount, Attack + 1);
                Test->TestTrue(TEXT("Recovery re-arms canonical action phase"),
                               C->Actions->GetPhase() == EPresentationPhase::Locomotion &&
                                   !A->bActionActive);
                Shot(*FString::Printf(TEXT("attack-%d-ready.png"), Attack + 1));
                if (++Attack < 4)
                {
                    Stage = 3;
                    Since = W->GetTimeSeconds();
                    Released = false;
                }
                else
                {
                    Key(PC, EKeys::E, IE_Pressed);
                    Next(W);
                }
            }
            break;
        case 6:
            if (!Released)
            {
                Key(PC, EKeys::E, IE_Released);
                Released = true;
            }
            if (T > .3)
            {
                Test->TestEqual(TEXT("Cultivate remains available after four sword recoveries"),
                                Commands->InputAttempts, 3);
                FFileHelper::SaveStringToFile(
                    Trace, *(FPaths::ProjectSavedDir() /
                             TEXT("ManualInputEvidence/offline-and-repeat-trace.txt")));
                return true;
            }
            break;
        }
        return false;
    }
};
bool FManualInputRegression::RunTest(const FString &)
{
    // This test intentionally exercises normal unconfigured Editor PIE. The
    // existing authority fixture runs separately, without weakening either case.
    if (FString(FCommandLine::Get()).Contains(TEXT("CultivationBridgeDir=")))
    {
        AddInfo(TEXT("Offline input fixture runs in its dedicated unconfigured process"));
        return true;
    }
    ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/DaoVoTan/Maps/StillwaterValley")));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitForShadersToFinishCompiling());
    ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
    ADD_LATENT_AUTOMATION_COMMAND(FManualInputExercise(this));
    ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FManualLiveCultivate, "DaoVoTan.Input.LiveCultivateAndSwordPIE",
                                 EAutomationTestFlags::EditorContext |
                                     EAutomationTestFlags::EngineFilter)
class FManualLiveExercise : public IAutomationLatentCommand
{
    FAutomationTestBase *Test;
    int32 Stage = 0;
    double Since = 0, Started = FPlatformTime::Seconds();
    FString ERequest;
    void Next(UWorld *W)
    {
        ++Stage;
        Since = W->GetTimeSeconds();
    }
    void Raw(AGatePlayerController *PC, FKey K, EInputEvent E)
    {
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(K, E, E == IE_Pressed ? 1.f : 0.f));
    }

  public:
    explicit FManualLiveExercise(FAutomationTestBase *T) : Test(T)
    {
    }
    bool Update() override
    {
        if (FPlatformTime::Seconds() - Started > 100)
        {
            Test->AddError(TEXT("Live manual-input timeout"));
            return true;
        }
        UWorld *W = nullptr;
        for (const auto &X : GEngine->GetWorldContexts())
            if (X.WorldType == EWorldType::PIE)
                W = X.World();
        auto *PC = W ? Cast<AGatePlayerController>(W->GetFirstPlayerController()) : nullptr;
        auto *C = PC ? Cast<AGateCharacter>(PC->GetPawn()) : nullptr;
        if (!C)
            return false;
        auto *Commands = PC->CultivationCommands.Get();
        auto *S =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
        const double T = W->GetTimeSeconds() - Since;
        switch (Stage)
        {
        case 0:
            if (W->GetTimeSeconds() > 3 && Commands->IsConnected())
            {
                Commands->SelectedDays = 7;
                // Explicit required InputAction injection, still through Enhanced Input's real
                // binding.
                S->InjectInputForAction(Commands->CultivateInputAction(), FInputActionValue(true),
                                        {}, {});
                Next(W);
            }
            break;
        case 1:
            if (!Commands->LastRequestId.IsEmpty())
            {
                ERequest = Commands->LastRequestId;
                Test->TestTrue(TEXT("E InputAction generates an actual command"),
                               Commands->InputAttempts == 1);
                Next(W);
            }
            break;
        case 2:
            if (Commands->Receipt.Status == TEXT("completed") && T > 5)
            {
                Test->TestEqual(TEXT("E completes via real worker once"),
                                Commands->LogicalResultCount, 1);
                FString Bridge, Json;
                FParse::Value(FCommandLine::Get(), TEXT("CultivationBridgeDir="), Bridge);
                FFileHelper::LoadFileToString(Json, *(Bridge / TEXT("session.json")));
                FFileHelper::SaveStringToFile(Json,
                                              *(FPaths::ProjectSavedDir() /
                                                TEXT("ManualInputEvidence/first-E-session.json")));
                Test->TestTrue(TEXT("Cultivate releases shared action before sword"),
                               C->Actions->GetPhase() == EPresentationPhase::Locomotion);
                Raw(PC, EKeys::LeftMouseButton, IE_Pressed);
                Next(W);
            }
            break;
        case 3:
            Raw(PC, EKeys::LeftMouseButton, IE_Released);
            if (T > .3)
            {
                Test->TestEqual(TEXT("Sword works after completed Cultivate presentation"),
                                C->Actions->SwordStartCount, 1);
                auto *A = Cast<UCultivatorLocomotion>(C->GetMesh()->GetAnimInstance());
                Test->TestTrue(TEXT("Sword sequence advances after Cultivate"),
                               A && A->ActionSequenceTime > .1f && A->ActionSequenceTime < 1.f);
                Next(W);
            }
            break;
        case 4:
            if (T > 1.7)
            {
                Test->TestTrue(TEXT("Sword released before M"),
                               C->Actions->GetPhase() == EPresentationPhase::Locomotion);
                Raw(PC, EKeys::M, IE_Pressed);
                Next(W);
            }
            break;
        case 5:
            Raw(PC, EKeys::M, IE_Released);
            if (Commands->LastRequestId != ERequest &&
                Commands->Receipt.Status == TEXT("completed") && T > 5)
            {
                Test->TestEqual(TEXT("Raw M maps to second authoritative command"),
                                Commands->InputAttempts, 2);
                Test->TestEqual(TEXT("E and M each complete once"), Commands->LogicalResultCount,
                                2);
                Test->TestTrue(TEXT("Second Cultivate releases shared gate"),
                               C->Actions->GetPhase() == EPresentationPhase::Locomotion);
                Test->TestTrue(
                    TEXT("Live input capture"),
                    CapturePIEEvidence(FPaths::ProjectSavedDir() /
                                       TEXT("ManualInputEvidence/live-E-M-completed.png")));
                Raw(PC, EKeys::LeftMouseButton, IE_Pressed);
                Next(W);
            }
            break;
        case 6:
            Raw(PC, EKeys::LeftMouseButton, IE_Released);
            if (T > .4)
            {
                Test->TestEqual(TEXT("Second sword works after M presentation"),
                                C->Actions->SwordStartCount, 2);
                auto *A = Cast<UCultivatorLocomotion>(C->GetMesh()->GetAnimInstance());
                Test->TestTrue(TEXT("Second sword sequence actually replays"),
                               A && A->ActionSequenceTime > .2f && A->ActionSequenceTime < 1.f);
                return true;
            }
            break;
        }
        return false;
    }
};
bool FManualLiveCultivate::RunTest(const FString &)
{
    if (!FParse::Param(FCommandLine::Get(), TEXT("ManualLiveTest")))
    {
        AddInfo(TEXT("Real worker input test runs in isolated -ManualLiveTest process"));
        return true;
    }
    ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/DaoVoTan/Maps/StillwaterValley")));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitForShadersToFinishCompiling());
    ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
    ADD_LATENT_AUTOMATION_COMMAND(FManualLiveExercise(this));
    ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
    return true;
}
#endif
