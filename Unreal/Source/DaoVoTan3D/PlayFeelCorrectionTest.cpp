#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "GateCharacter.h"
#include "CultivatorLocomotion.h"
#include "PresentationActions.h"
#include "PresentationEvidence.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/FileHelper.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPlayFeelCorrection, "DaoVoTan.Correction.ManualSequencePIE",
                                 EAutomationTestFlags::EditorContext |
                                     EAutomationTestFlags::EngineFilter)
class FCorrectionSequence : public IAutomationLatentCommand
{
    FAutomationTestBase *Test;
    int32 Stage = 0;
    double Since = 0, Started = FPlatformTime::Seconds();
    FString Trace = TEXT("time,stage,grounded,speed,accel,input,controllerYaw,actorYaw,velocityYaw,"
                         "cameraYaw,db,clip,landingRemaining,phase,contactL,contactR,handX,handY,"
                         "handZ,tipX,tipY,tipZ,sequenceTime,actionTime,actionBlend,elbowX,elbowY,"
                         "elbowZ,shoulderX,shoulderY,shoulderZ,pelvisX,pelvisY,pelvisZ,headX,headY,"
                         "headZ,thighLX,thighLY,thighLZ,thighRX,thighRY,thighRZ\n");
    FString Label;
    TSet<FString> Shots;
    bool WasGrounded = true;
    double ContactTime = -1;
    bool WalkRecovered = false, RunRecovered = false;
    int32 ExtraCase = 0;
    double Elapsed(UWorld *W) const
    {
        return W->GetTimeSeconds() - Since;
    }
    void Next(UWorld *W)
    {
        ++Stage;
        Since = W->GetTimeSeconds();
    }
    void Input(AGateCharacter *C, FKey Key)
    {
        auto *PC = Cast<APlayerController>(C->GetController());
        auto *S =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
        for (const auto &M : C->GetPresentationInputContext()->GetMappings())
            if (M.Key == Key)
            {
                TArray<UInputModifier *> Mods;
                for (auto P : M.Modifiers)
                    Mods.Add(P.Get());
                S->InjectInputForAction(M.Action, FInputActionValue(1.f), Mods, {});
                break;
            }
    }
    void Shot(const FString &Name)
    {
        if (!Shots.Contains(Name))
        {
            Shots.Add(Name);
            Test->TestTrue(
                TEXT("Correction PIE evidence"),
                CapturePIEEvidence(FPaths::ProjectSavedDir() / TEXT("CorrectionEvidence") / Label /
                                   (Name + TEXT(".png"))));
        }
    }

  public:
    explicit FCorrectionSequence(FAutomationTestBase *T) : Test(T)
    {
        if (!FParse::Value(FCommandLine::Get(), TEXT("CorrectionLabel="), Label))
            Label = TEXT("probe");
    }
    bool Update() override
    {
        if (FPlatformTime::Seconds() - Started > 180)
        {
            Test->AddError(TEXT("Correction sequence timeout"));
            return true;
        }
        UWorld *W = nullptr;
        for (const auto &Context : GEngine->GetWorldContexts())
            if (Context.WorldType == EWorldType::PIE)
                W = Context.World();
        if (!W || !W->GetFirstPlayerController())
            return false;
        auto *PC = W->GetFirstPlayerController();
        auto *C = Cast<AGateCharacter>(PC->GetPawn());
        if (!C)
            return false;
        auto *A = Cast<UCultivatorLocomotion>(C->GetMesh()->GetAnimInstance());
        if (!A)
            return false;
        auto *M = C->GetCharacterMovement();
        // Widen only the evidence view, as in the accepted animation test.
        if (!C->Actions->IsMeditating())
            C->Boom->TargetArmLength = 550;
        const double T = Elapsed(W);
        if ((Stage == 3 || Stage == 6) && !WasGrounded && M->IsMovingOnGround())
            ContactTime = W->GetTimeSeconds();
        WasGrounded = M->IsMovingOnGround();
        if (ContactTime >= 0 && (Stage == 3 || Stage == 4 || Stage == 6 || Stage == 7))
        {
            const bool Run = Stage >= 6;
            if (A->SelectedDatabase == (Run ? TEXT("PSD_Run") : TEXT("PSD_Walk")) &&
                A->SelectedAnimation.StartsWith(Run ? TEXT("M_Neutral_Run_")
                                                    : TEXT("M_Neutral_Walk_")))
            {
                const double Latency = W->GetTimeSeconds() - ContactTime;
                Test->AddInfo(
                    FString::Printf(TEXT("Measured %s actual-clip landing recovery %.6f seconds"),
                                    Run ? TEXT("run") : TEXT("walk"), Latency));
                Test->TestTrue(TEXT("Landing eliminates the measured old 0.45-second hold"),
                               Latency < .40);
                if (Run)
                    RunRecovered = true;
                else
                    WalkRecovered = true;
                ContactTime = -1;
            }
        }
        const FVector Hand = C->GetMesh()->GetSocketLocation(TEXT("hand_r"));
        const FVector Tip = C->Blade->GetComponentTransform().TransformPosition(FVector(0, 0, 101));
        Trace += FString::Printf(
            TEXT("%.6f,%d,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%s,%s,%.6f,%d,%.4f,%.4f,%.3f,%.3f,%"
                 ".3f,%.3f,%.3f,%.3f\n"),
            W->GetTimeSeconds(), Stage, M->IsMovingOnGround(), C->GetVelocity().Size2D(),
            M->GetCurrentAcceleration().Size2D(), C->GetLastMovementInputVector().Size2D(),
            PC->GetControlRotation().Yaw, C->GetActorRotation().Yaw,
            C->GetVelocity().Rotation().Yaw, C->Camera->GetComponentRotation().Yaw,
            *A->SelectedDatabase, *A->SelectedAnimation, A->LandingAge,
            int32(C->Actions->GetPhase()), A->GetCurveValue(TEXT("contact_l")),
            A->GetCurveValue(TEXT("contact_r")), Hand.X, Hand.Y, Hand.Z, Tip.X, Tip.Y, Tip.Z);
        Trace.RemoveAt(Trace.Len() - 1);
        Trace += FString::Printf(TEXT(",%.6f,%.6f,%.6f"), A->SelectedSequenceTime,
                                 A->ActionSequenceTime, A->ActionBlendWeight);
        for (const TCHAR *Bone : {TEXT("lowerarm_r"), TEXT("upperarm_r"), TEXT("pelvis"),
                                  TEXT("head"), TEXT("thigh_l"), TEXT("thigh_r")})
        {
            const FVector P = C->GetMesh()->GetSocketLocation(Bone);
            Trace += FString::Printf(TEXT(",%.3f,%.3f,%.3f"), P.X, P.Y, P.Z);
        }
        Trace += TEXT("\n");
        for (const TCHAR *Bone : {TEXT("foot_l"), TEXT("foot_r"), TEXT("calf_l"), TEXT("calf_r")})
        {
            const FVector P = C->GetMesh()->GetSocketLocation(Bone);
            Test->TestFalse(TEXT("IK bone transforms remain finite"), P.ContainsNaN());
            Test->TestTrue(TEXT("IK leg remains within character reach"),
                           FVector::Dist(P, C->GetActorLocation()) < 240);
        }
        switch (Stage)
        {
        case 0:
            C->SetActorLocation(FVector(-11500, 0, 110));
            PC->SetControlRotation(FRotator(-12, 0, 0));
            Next(W);
            break;
        case 1:
            if (Label != TEXT("before"))
            {
                PC->SetControlRotation(FRotator(-10, T < 1 ? 180 : T < 2 ? 90 : -45, 0));
                if (T > .7 && T < 1)
                    Shot(TEXT("01-idle-front"));
                if (T > 1.7 && T < 2)
                    Shot(TEXT("02-idle-side"));
                if (T > 2.7)
                    Shot(TEXT("03-idle-rear-quarter"));
            }
            if (T > 3)
            {
                Shot(TEXT("01-idle"));
                PC->SetControlRotation(FRotator(-12, 0, 0));
                Next(W);
            }
            break;
        case 2:
            Input(C, EKeys::W);
            if (T > 1.2)
            {
                C->TryJump();
                Next(W);
            }
            break;
        case 3:
            Input(C, EKeys::W);
            if (T > .25 && M->IsMovingOnGround())
            {
                Shot(TEXT("walk-contact"));
                Next(W);
            }
            break;
        case 4:
            Input(C, EKeys::W);
            if (T > .8)
            {
                Shot(TEXT("walk-recovery"));
                C->ToggleRun();
                Next(W);
            }
            break;
        case 5:
            Input(C, EKeys::W);
            if (T > 1.2)
            {
                C->TryJump();
                Next(W);
            }
            break;
        case 6:
            Input(C, EKeys::W);
            if (T > .25 && M->IsMovingOnGround())
            {
                Shot(TEXT("run-contact"));
                Next(W);
            }
            break;
        case 7:
            Input(C, EKeys::W);
            if (T > .8)
            {
                Shot(TEXT("run-recovery"));
                Next(W);
            }
            break;
        case 8:
            Input(C, EKeys::W);
            Input(C, EKeys::D);
            PC->AddYawInput(.35f);
            if (T > 1.2)
            {
                Shot(TEXT("diagonal-camera"));
                Next(W);
            }
            break;
        case 9:
            Input(C, EKeys::S);
            if (T > .25)
                Shot(TEXT("pivot"));
            if (T > 1)
                Next(W);
            break;
        case 10:
            if (T > 1.5)
            {
                PC->SetControlRotation(FRotator(-10, C->GetActorRotation().Yaw + 135, 0));
                C->Actions->PlaySwordPresentation();
                Next(W);
            }
            break;
        case 11:
            for (int32 I = 0; I <= 12; ++I)
                if (T >= I * .1)
                    Shot(FString::Printf(TEXT("attack-%02d"), I));
            if (T > 1.7)
                Next(W);
            break;
        case 12:
            Input(C, EKeys::W);
            if (T > 1)
            {
                C->Actions->PlaySwordPresentation();
                Next(W);
            }
            break;
        case 13:
            Input(C, EKeys::W);
            if (T > .4)
                Shot(TEXT("moving-attack"));
            if (T > 1.8)
                Next(W);
            break;
        case 14:
            if (T > 1)
            {
                C->Actions->StartMeditationPresentation();
                Next(W);
            }
            break;
        case 15:
            if (T > 1.8)
            {
                Shot(TEXT("cultivation-presentation"));
                C->Actions->StopMeditationPresentation();
                Next(W);
            }
            break;
        case 16:
            if (T > 1.5)
            {
                Next(W);
            }
            break;
        case 17:
            // Cover all four diagonals plus forward and strafe with continuous yaw.
            Input(C, ExtraCase >= 3 && ExtraCase <= 4 ? EKeys::S : EKeys::W);
            if (ExtraCase == 1 || ExtraCase == 3)
                Input(C, EKeys::A);
            if (ExtraCase == 2 || ExtraCase == 4)
                Input(C, EKeys::D);
            PC->AddYawInput(.35f);
            if (T > .7)
            {
                Shot(FString::Printf(TEXT("camera-direction-%d"), ExtraCase));
                if (++ExtraCase == 5)
                    C->ToggleStrafe();
                if (ExtraCase == 6)
                {
                    C->ToggleStrafe();
                    ExtraCase = 0;
                    Next(W);
                }
                else
                    Since = W->GetTimeSeconds();
            }
            break;
        case 18:
            // Slow walk, diagonal jump, short jump and a longer fall.
            M->MaxWalkSpeed = ExtraCase == 0 ? 120 : ExtraCase == 1 ? 600 : 240;
            if (ExtraCase < 2)
            {
                Input(C, EKeys::W);
                if (ExtraCase == 1)
                    Input(C, EKeys::D);
            }
            if (T > .8)
            {
                M->JumpZVelocity = ExtraCase == 2 ? 260 : 500;
                if (ExtraCase == 3)
                {
                    C->SetActorLocation(C->GetActorLocation() + FVector(0, 0, 400));
                    M->SetMovementMode(MOVE_Falling);
                }
                else
                    C->TryJump();
                Next(W);
            }
            break;
        case 19:
            if (ExtraCase < 2)
            {
                Input(C, EKeys::W);
                if (ExtraCase == 1)
                    Input(C, EKeys::D);
            }
            if (T > .4 && M->IsMovingOnGround())
            {
                Shot(FString::Printf(TEXT("extra-landing-%d"), ExtraCase));
                Next(W);
            }
            break;
        case 20:
            if (ExtraCase < 2)
            {
                Input(C, EKeys::W);
                if (ExtraCase == 1)
                    Input(C, EKeys::D);
            }
            if (T > .7)
            {
                if (++ExtraCase < 4)
                {
                    Stage = 18;
                    Since = W->GetTimeSeconds();
                }
                else
                {
                    M->JumpZVelocity = 500;
                    Next(W);
                }
            }
            break;
        case 21:
            if (T > 1)
            {
                Test->TestTrue(TEXT("Locomotion returns after sequence"),
                               C->Actions->GetPhase() == EPresentationPhase::Locomotion);
                Test->TestTrue(TEXT("Walk landing actually returns to walk clip"), WalkRecovered);
                Test->TestTrue(TEXT("Run landing actually returns to run clip"), RunRecovered);
                const FString File = FPaths::ProjectSavedDir() / TEXT("CorrectionEvidence") /
                                     Label / TEXT("trace.csv");
                Test->TestTrue(TEXT("Correction trace saved"),
                               FFileHelper::SaveStringToFile(Trace, *File));
                return true;
            }
            break;
        }
        return false;
    }
};
bool FPlayFeelCorrection::RunTest(const FString &)
{
    ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/DaoVoTan/Maps/StillwaterValley")));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitForShadersToFinishCompiling());
    ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
    ADD_LATENT_AUTOMATION_COMMAND(FCorrectionSequence(this));
    ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
    return true;
}
#endif
