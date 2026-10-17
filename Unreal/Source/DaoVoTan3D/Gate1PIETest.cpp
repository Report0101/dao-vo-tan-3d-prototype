#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS
#include "AppearanceComponent.h"
#include "AuraComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GateCharacter.h"
#include "GateHUD.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "PresentationActions.h"
#include "Tests/AutomationEditorCommon.h"
#include "UnrealClient.h"
#include "PresentationEvidence.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGate1PIETest, "DaoVoTan.Gate1.PlayableSlice",
                                 EAutomationTestFlags::EditorContext |
                                     EAutomationTestFlags::EngineFilter)
class FGate1Exercise : public IAutomationLatentCommand
{
    FAutomationTestBase *Test;
    int32 Stage = 0;
    double Since = 0, Started = 0;
    FVector Initial;
    int32 Frames = 0;
    double SumFrame = 0, SumGame = 0, SumGPU = 0;
    int32 Samples = 0;
    UWorld *World()
    {
        for (const auto &C : GEngine->GetWorldContexts())
            if (C.WorldType == EWorldType::PIE)
                return C.World();
        return nullptr;
    }
    FString Dir()
    {
        return FPaths::Combine(FPaths::ProjectSavedDir(),
                               FParse::Param(FCommandLine::Get(), TEXT("GateStress"))
                                   ? TEXT("Gate1Evidence-Stress")
                                   : TEXT("Gate1Evidence"));
    }
    void Shot(const TCHAR *Name)
    {
        if (!CapturePIEEvidence(FPaths::Combine(Dir(), Name)))
            Test->AddError(TEXT("PIE evidence capture failed"));
    }
    void Next()
    {
        ++Stage;
        Since = FPlatformTime::Seconds();
    }
    void Inject(AGateCharacter *C, FKey Key, float V)
    {
        auto *PC = Cast<APlayerController>(C->GetController());
        if (!PC || !PC->GetLocalPlayer())
            return;
        auto *S =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
        auto *Context = C->GetPresentationInputContext();
        if (!S || !Context)
            return;
        for (auto &M : Context->GetMappings())
            if (M.Key == Key)
            {
                TArray<UInputModifier *> Modifiers;
                for (auto Modifier : M.Modifiers)
                    Modifiers.Add(Modifier.Get());
                S->InjectInputForAction(M.Action, FInputActionValue(V), Modifiers, {});
                break;
            }
    }

  public:
    explicit FGate1Exercise(FAutomationTestBase *T) : Test(T)
    {
        Started = Since = FPlatformTime::Seconds();
    }
    virtual bool Update() override
    {
        auto *W = World();
        double Now = FPlatformTime::Seconds();
        if (Now - Started > 180)
        {
            Test->AddError(TEXT("Timed out exercising Gate 1 PIE"));
            return true;
        }
        if (!W || !W->GetFirstPlayerController())
            return false;
        auto *PC = W->GetFirstPlayerController();
        auto *C = Cast<AGateCharacter>(PC->GetPawn());
        if (!C)
            return false;
        double T = Now - Since;
        switch (Stage)
        {
        case 0:
            if (T < 4)
                return false;
            // The movement assertions below use world X. Physical mouse look
            // must not rotate that reference during injected input playback.
            PC->SetIgnoreLookInput(true);
            PC->SetControlRotation(FRotator(-12, 0, 0));
            Test->TestTrue(TEXT("PIE world is running"), W->HasBegunPlay());
            Test->TestNotNull(TEXT("HUD"), Cast<AGateHUD>(PC->GetHUD()));
            Test->TestTrue(TEXT("Camera collision probe enabled"), C->Boom->bDoCollisionTest);
            Test->TestNotNull(TEXT("Player skeletal mesh"), C->GetMesh()->GetSkeletalMeshAsset());
            {
                int32 Count = 0;
                for (TActorIterator<AGateCharacter> I(W); I; ++I)
                {
                    ++Count;
                    Test->TestTrue(TEXT("Shared skeleton family"),
                                   I->GetMesh()->GetSkeletalMeshAsset()->GetSkeleton() ==
                                       C->GetMesh()->GetSkeletalMeshAsset()->GetSkeleton());
                }
                int32 Expected = 3;
                if (FParse::Param(FCommandLine::Get(), TEXT("GateStress")))
                    Expected = 11;
                Test->TestEqual(TEXT("Visible fixture character count"), Count, Expected);
            }
            PC->ConsoleCommand(TEXT("stat unit"));
            Shot(TEXT("01-valley-npcs-ui.png"));
            Initial = C->GetActorLocation();
            Next();
            break;
        case 1:
            Inject(C, EKeys::W, 1);
            if (T > 1.5f)
            {
                Test->TestTrue(TEXT("Enhanced Input moves player"),
                               FVector::Dist2D(Initial, C->GetActorLocation()) > 150);
                Shot(TEXT("02-walk.png"));
                C->ToggleRun();
                Next();
            }
            break;
        case 2:
            Inject(C, EKeys::W, 1);
            if (T > 1)
            {
                Test->TestTrue(TEXT("Run faster than walk"), C->GetVelocity().Size2D() > 400);
                C->TryJump();
                Next();
            }
            break;
        case 3:
            if (T > .2f)
            {
                Test->TestTrue(TEXT("Jump becomes airborne"),
                               C->GetCharacterMovement()->IsFalling());
                Shot(TEXT("03-jump.png"));
                Next();
            }
            break;
        case 4:
            if (T > 2)
            {
                C->SwordAction();
                Next();
            }
            break;
        case 5:
            if (T > .25f)
            {
                Test->TestTrue(TEXT("Sword visual action active"),
                               C->Actions->GetPhase() == EPresentationPhase::Sword);
                Test->TestTrue(TEXT("Sword attached to weapon socket"),
                               C->Blade->GetAttachParent() == C->WeaponSocket);
                Shot(TEXT("04-sword.png"));
                Next();
            }
            break;
        case 6:
            if (T > 2)
            {
                C->Actions->StartMeditationPresentation();
                Next();
            }
            break;
        case 7:
            if (T > 2)
            {
                Test->TestTrue(TEXT("Meditation loop active"),
                               C->Actions->GetPhase() == EPresentationPhase::MeditationLoop);
                Test->TestTrue(TEXT("Movement disabled during meditation"),
                               C->GetCharacterMovement()->MovementMode == MOVE_None);
                Test->TestTrue(TEXT("Aura enabled"), C->Aura->IsEmitting());
                Shot(TEXT("05-meditation-jade.png"));
                Next();
            }
            break;
        case 8:
            if (T > 1)
            {
                C->CycleAura();
                Shot(TEXT("06-aura-ember.png"));
                Next();
            }
            break;
        case 9:
            if (T > 1)
            {
                C->CycleAura();
                Shot(TEXT("07-aura-violet.png"));
                Next();
            }
            break;
        case 10:
            if (T > 1)
            {
                C->Actions->StopMeditationPresentation();
                Next();
            }
            break;
        case 11:
            if (T > 2)
            {
                Test->TestTrue(TEXT("Movement restored on exit"),
                               C->GetCharacterMovement()->MovementMode == MOVE_Walking);
                Test->TestFalse(TEXT("Aura stopped on exit"), C->Aura->IsEmitting());
                PC->SetControlRotation(FRotator(-12, 0, 0));
                Initial = C->GetActorLocation();
                Next();
            }
            break;
        case 12:
            Inject(C, EKeys::S, 1);
            if (T > 1)
            {
                Test->TestTrue(TEXT("Back mapping moves backwards"),
                               C->GetActorLocation().X < Initial.X - 100);
                C->SetActorLocation(FVector(-12500, 0, 100));
                PC->SetControlRotation(FRotator(-12, 0, 0));
                Next();
            }
            break;
        case 13:
            ++Frames;
            SumFrame += FApp::GetDeltaTime() * 1000;
            if (auto *Data = W->GetGameViewport()->GetStatUnitData())
            {
                SumGame += Data->GameThreadTime;
                SumGPU += Data->GPUFrameTime[0];
                ++Samples;
            }
            if (T > 8)
            {
                FString Summary = FString::Printf(
                    TEXT("mode=EditorPIE\nframes=%d\nfps=%.2f\nframe_ms=%.3f\ngame_thread_ms=%."
                         "3f\ngpu_ms=%."
                         "3f\naura_backend=Niagara_shared_system\n"),
                    Frames, Frames * 1000 / SumFrame, SumFrame / Frames,
                    SumGame / FMath::Max(Samples, 1), SumGPU / FMath::Max(Samples, 1));
                int32 Actors = 0, Visible = 0, ActiveAura = 0;
                for (TActorIterator<AGateCharacter> I(W); I; ++I)
                {
                    ++Actors;
                    if (I->Appearance->Garment && I->Appearance->Garment->WasRecentlyRendered(.3f))
                        ++Visible;
                    if (I->Aura->IsEmitting())
                        ++ActiveAura;
                }
                Summary += FString::Printf(
                    TEXT("skeletal_actors=%d\nrecently_rendered_skeletal_actors=%d\n"), Actors,
                    Visible);
                Summary += FString::Printf(TEXT("active_Niagara_instances=%d\n"), ActiveAura);
                FFileHelper::SaveStringToFile(Summary,
                                              *FPaths::Combine(Dir(), TEXT("performance.txt")));
                Test->AddInfo(Summary);
                Shot(TEXT("08-after-exit.png"));
                Next();
            }
            break;
        default:
            return T > 1;
        }
        return false;
    }
};
bool FGate1PIETest::RunTest(const FString &Params)
{
    ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/DaoVoTan/Maps/StillwaterValley")));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitForShadersToFinishCompiling());
    ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
    ADD_LATENT_AUTOMATION_COMMAND(FGate1Exercise(this));
    ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
    return true;
}
#endif
