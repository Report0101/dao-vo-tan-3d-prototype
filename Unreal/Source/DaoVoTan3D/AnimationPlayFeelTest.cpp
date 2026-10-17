#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimData/IAnimationDataModel.h"
#include "Animation/AnimCurveTypes.h"
#include "Animation/AnimInstance.h"
#include "AppearanceComponent.h"
#include "AuraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "CultivatorLocomotion.h"
#include "Engine/SkeletalMesh.h"
#include "EngineUtils.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GateCharacter.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Misc/FileHelper.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "PresentationActions.h"
#include "PresentationEvidence.h"
#include "Tests/AutomationEditorCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAnimationAssetContract, "DaoVoTan.Animation.AssetContract",
                                 EAutomationTestFlags::EditorContext |
                                     EAutomationTestFlags::EngineFilter)
bool FAnimationAssetContract::RunTest(const FString &)
{
    auto *Mesh = LoadObject<USkeletalMesh>(
        nullptr, TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple"));
    int32 Count = 0;
    bool RootTravel = false;
    for (const TCHAR *State :
         {TEXT("Idle"), TEXT("Turn"), TEXT("Walk"), TEXT("Run"), TEXT("Air"), TEXT("Landing")})
    {
        auto *DB = LoadObject<UPoseSearchDatabase>(
            nullptr, *(FString(TEXT("/Game/DaoVoTan/Locomotion/PSD_")) + State));
        if (!TestNotNull(TEXT("Scoped Pose Search database"), DB))
            continue;
        TestTrue(TEXT("Database populated"), DB->GetNumAnimationAssets() > 0);
        for (int32 I = 0; I < DB->GetNumAnimationAssets(); ++I)
        {
            const auto *Entry = DB->GetDatabaseAnimationAsset(I);
            auto *Clip = Entry ? Cast<UAnimSequence>(Entry->GetAnimationAsset()) : nullptr;
            if (!TestNotNull(TEXT("Retargeted sequence"), Clip))
                continue;
            ++Count;
            TestTrue(TEXT("Original Manny skeleton retained"),
                     Clip->GetSkeleton() == Mesh->GetSkeleton());
            TestTrue(TEXT("No sample gameplay/foley notifies migrated"), Clip->Notifies.IsEmpty());
            bool LeftContact = false, RightContact = false;
            for (const auto &Curve : Clip->GetDataModel()->GetFloatCurves())
            {
                LeftContact |= Curve.GetName() == TEXT("contact_l");
                RightContact |= Curve.GetName() == TEXT("contact_r");
            }
            TestTrue(TEXT("Both sample foot contact curves preserved"),
                     LeftContact && RightContact);
            TestTrue(TEXT("Clip belongs to bounded destination"),
                     Clip->GetPathName().StartsWith(TEXT("/Game/DaoVoTan/Locomotion/Clips/")));
            if (FString(State) == TEXT("Run") &&
                Clip->ExtractRootMotionFromRange(0, Clip->GetPlayLength(), FAnimExtractContext())
                        .GetTranslation()
                        .Size2D() > 100)
                RootTravel = true;
        }
    }
    TestEqual(TEXT("Explicit minimal selection count"), Count, 86);
    TestTrue(TEXT("Root travel retained for trajectory matching"), RootTravel);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAnimationPlayFeel, "DaoVoTan.Animation.PlayFeelPIE",
                                 EAutomationTestFlags::EditorContext |
                                     EAutomationTestFlags::EngineFilter)
class FAnimationExercise : public IAutomationLatentCommand
{
    FAutomationTestBase *Test;
    int32 Stage = 0;
    double Since = 0, Started = FPlatformTime::Seconds();
    FString Trace;
    FVector Start;
    void Next()
    {
        ++Stage;
        Since = FPlatformTime::Seconds();
    }
    void Inject(AGateCharacter *C, FKey Key)
    {
        auto *PC = Cast<APlayerController>(C->GetController());
        auto *S =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
        for (const auto &M : C->GetPresentationInputContext()->GetMappings())
            if (M.Key == Key)
            {
                TArray<UInputModifier *> Modifiers;
                for (auto P : M.Modifiers)
                    Modifiers.Add(P.Get());
                S->InjectInputForAction(M.Action, FInputActionValue(1.f), Modifiers, {});
                break;
            }
    }
    void Shot(UCultivatorLocomotion *Anim, const TCHAR *Name)
    {
        Test->TestTrue(TEXT("Actual motion-matched clip selected"),
                       !Anim->SelectedAnimation.IsEmpty() &&
                           Anim->SelectedAnimation != TEXT("None"));
        Trace += FString::Printf(
            TEXT("%s db=%s clip=%s speed=%.2f air=%d trajectory=%d selections=%d\n"), Name,
            *Anim->SelectedDatabase, *Anim->SelectedAnimation, Anim->Speed, Anim->bAirborne,
            Anim->Trajectory.Samples.Num(), Anim->PoseSelections);
        Test->TestTrue(TEXT("PIE capture"), CapturePIEEvidence(FPaths::ProjectSavedDir() /
                                                               TEXT("AnimationEvidence") / Name));
    }

  public:
    explicit FAnimationExercise(FAutomationTestBase *T) : Test(T)
    {
    }
    bool Update() override
    {
        if (FPlatformTime::Seconds() - Started > 160)
        {
            Test->AddError(TEXT("Animation PIE timeout"));
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
        auto *Anim = Cast<UCultivatorLocomotion>(C->GetMesh()->GetAnimInstance());
        if (!Anim)
        {
            Test->AddError(TEXT("Native locomotion instance missing"));
            return true;
        }
        // Widen only this automated pose-inspection view so boots and foot
        // contacts remain visible above the HUD. Normal gameplay camera is unchanged.
        if (!C->Actions->IsMeditating())
            C->Boom->TargetArmLength = 550;
        const double T = FPlatformTime::Seconds() - Since;
        switch (Stage)
        {
        case 0:
            PC->SetIgnoreLookInput(true);
            PC->SetControlRotation(FRotator(-12, 0, 0));
            C->SetActorLocation(FVector(-11500, 0, 110));
            Start = C->GetActorLocation();
            Next();
            break;
        case 1:
            if (T > 4)
            {
                for (FName Foot : {FName(TEXT("foot_l")), FName(TEXT("foot_r"))})
                {
                    const FVector P = C->GetMesh()->GetSocketLocation(Foot);
                    FHitResult Hit;
                    FCollisionQueryParams Query(SCENE_QUERY_STAT(AnimationFootEvidence), false, C);
                    if (Test->TestTrue(TEXT("Floor beneath foot"),
                                       W->LineTraceSingleByChannel(Hit, P + FVector(0, 0, 80),
                                                                   P - FVector(0, 0, 120),
                                                                   ECC_Visibility, Query)))
                    {
                        const float Height = P.Z - Hit.ImpactPoint.Z;
                        Trace += FString::Printf(TEXT("idle %s ankle_height_cm=%.3f\n"),
                                                 *Foot.ToString(), Height);
                        Test->TestTrue(TEXT("Idle ankle remains near floor"),
                                       Height > -10 && Height < 35);
                    }
                }
                Shot(Anim, TEXT("01-idle.png"));
                Next();
            }
            break;
        case 2:
            Inject(C, EKeys::W);
            if (T > .3)
            {
                Shot(Anim, TEXT("02-start.png"));
                Next();
            }
            break;
        case 3:
            Inject(C, EKeys::W);
            if (T > 1.4)
            {
                Shot(Anim, TEXT("03-walk.png"));
                C->ToggleRun();
                Next();
            }
            break;
        case 4:
            Inject(C, EKeys::W);
            if (T > 1)
            {
                Shot(Anim, TEXT("04-run.png"));
                Test->TestTrue(TEXT("Run gait selects run clip"),
                               Anim->SelectedAnimation.Contains(TEXT("_Run_")));
                Next();
            }
            break;
        case 5:
            Inject(C, EKeys::S);
            if (T > .3)
            {
                Shot(Anim, TEXT("05-pivot.png"));
                Next();
            }
            break;
        case 6:
            if (T > .4)
            {
                Shot(Anim, TEXT("06-stop.png"));
                C->ToggleRun();
                C->ToggleStrafe();
                Next();
            }
            break;
        case 7:
            Inject(C, EKeys::A);
            if (T > 1)
            {
                Shot(Anim, TEXT("07-strafe.png"));
                Test->TestTrue(TEXT("Omnidirectional movement keeps facing"),
                               C->IsStrafing() && FMath::Abs(C->GetActorRotation().Yaw) < 5);
                Test->TestTrue(TEXT("Strafe velocity is lateral"), C->GetVelocity().Y < -100);
                Next();
            }
            break;
        case 8:
            if (T > .8)
            {
                PC->SetControlRotation(FRotator(-12, 120, 0));
                Next();
            }
            break;
        case 9:
            if (T > .15)
            {
                Shot(Anim, TEXT("08-turn-in-place.png"));
                Next();
            }
            break;
        case 10:
            if (T > .8)
            {
                C->ToggleStrafe();
                C->TryJump();
                Next();
            }
            break;
        case 11:
            if (T > .2)
            {
                Shot(Anim, TEXT("09-jump.png"));
                Test->TestTrue(TEXT("Jump airborne"), Anim->bAirborne);
                Next();
            }
            break;
        case 12:
            if (T > .42)
            {
                Shot(Anim, TEXT("10-fall.png"));
                Next();
            }
            break;
        case 13:
            if (Anim->bLanding && Anim->SelectedAnimation.Contains(TEXT("_Land_")))
            {
                Shot(Anim, TEXT("11-landing.png"));
                Next();
            }
            else if (T > 3)
            {
                Test->AddError(TEXT("Landing state not observed"));
                Next();
            }
            break;
        case 14:
            if (T > 1)
            {
                for (const TCHAR *State : {TEXT("PSD_Idle"), TEXT("PSD_Walk"), TEXT("PSD_Run"),
                                           TEXT("PSD_Turn"), TEXT("PSD_Air"), TEXT("PSD_Landing")})
                    Test->TestTrue(FString(TEXT("Chooser visited ")) + State,
                                   Anim->SeenDatabases.Contains(State));
                Test->TestTrue(TEXT("Actual pose selections change during input"),
                               Anim->PoseSelections > 4);
                Test->TestTrue(TEXT("Trajectory contains history and prediction"),
                               Anim->Trajectory.Samples.Num() > 10);
                Test->TestTrue(TEXT("Movement remained CharacterMovement driven"),
                               FVector::Dist2D(Start, C->GetActorLocation()) > 150);
                C->SwordAction();
                Next();
            }
            break;
        case 15:
            if (T > .4)
            {
                Shot(Anim, TEXT("12-jian.png"));
                Next();
            }
            break;
        case 16:
            if (T > 1.3)
            {
                C->Actions->StartMeditationPresentation();
                Next();
            }
            break;
        case 17:
            if (T > 1.8)
            {
                Test->TestTrue(TEXT("Meditation suppression retained"),
                               C->GetCharacterMovement()->MovementMode == MOVE_None);
                Test->TestTrue(TEXT("Cultivation aura retained"), C->Aura->IsEmitting());
                Shot(Anim, TEXT("13-meditation.png"));
                C->Actions->StopMeditationPresentation();
                Next();
            }
            break;
        case 18:
            if (T > 1.5)
            {
                Test->TestTrue(TEXT("Locomotion resumes after meditation"),
                               C->Actions->GetPhase() == EPresentationPhase::Locomotion &&
                                   C->GetCharacterMovement()->MovementMode == MOVE_Walking);
                Test->TestTrue(TEXT("Animation cannot drive capsule root motion"),
                               Anim->UsesCosmeticRootMotion());
                FFileHelper::SaveStringToFile(
                    Trace, *(FPaths::ProjectSavedDir() / TEXT("AnimationEvidence/pose-trace.txt")));
                Test->AddInfo(Trace);
                return true;
            }
            break;
        }
        return false;
    }
};
bool FAnimationPlayFeel::RunTest(const FString &)
{
    ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/DaoVoTan/Maps/StillwaterValley")));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitForShadersToFinishCompiling());
    ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
    ADD_LATENT_AUTOMATION_COMMAND(FAnimationExercise(this));
    ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
    return true;
}
#endif
