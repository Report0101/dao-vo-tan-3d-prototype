#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS
#include "Animation/AnimSequence.h"
#include "AppearanceComponent.h"
#include "AuraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/SpringArmComponent.h"
#include "GateCharacter.h"
#include "ImageUtils.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "PortraitComponent.h"
#include "PresentationActions.h"
#include "PresentationRecipe.h"
#include "Serialization/BufferArchive.h"
#include "Tests/AutomationEditorCommon.h"
#include "UnrealClient.h"
#include "PresentationEvidence.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVisual11Assets, "DaoVoTan.Visual11.AssetAndSeedContract",
                                 EAutomationTestFlags::EditorContext |
                                     EAutomationTestFlags::EngineFilter)
bool FVisual11Assets::RunTest(const FString &Params)
{
    auto *Rig = LoadObject<USkeletalMesh>(
        nullptr, TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple"));
    for (int32 I = 0; I < 3; ++I)
    {
        auto *Garment = LoadObject<USkeletalMesh>(
            nullptr, *FString::Printf(TEXT("/Game/DaoVoTan/Visual11/SK_Robe%d"), I));
        TestNotNull(TEXT("Skinned garment"), Garment);
        if (Garment)
            TestTrue(TEXT("Same skeleton family"), Garment->GetSkeleton() == Rig->GetSkeleton());
        TestNotNull(TEXT("Head variant"),
                    LoadObject<UStaticMesh>(
                        nullptr, *FString::Printf(TEXT("/Game/DaoVoTan/Visual11/SM_Head%d"), I)));
    }
    for (int32 I = 0; I < 4; ++I)
        TestNotNull(TEXT("Hair variant"),
                    LoadObject<UStaticMesh>(
                        nullptr, *FString::Printf(TEXT("/Game/DaoVoTan/Visual11/SM_Hair%d"), I)));
    for (int32 I = 0; I < 2; ++I)
        TestNotNull(
            TEXT("Accessory"),
            LoadObject<UStaticMesh>(
                nullptr, *FString::Printf(TEXT("/Game/DaoVoTan/Visual11/SM_Accessory%d"), I)));
    auto *System =
        LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/DaoVoTan/Visual11/NS_CultivationAura"));
    TestNotNull(TEXT("One reusable aura asset"), System);
    if (System)
        TestEqual(TEXT("One emitter architecture"), System->GetEmitterHandles().Num(), 1);
    FString SeedProof;
    for (const TCHAR *Seed : {TEXT("01020304"), TEXT("a5371f2b"), TEXT("ffffffff")})
    {
        auto *A = NewObject<UPresentationRecipe>();
        auto *B = NewObject<UPresentationRecipe>();
        A->ApplyAppearanceSeed(Seed);
        B->ApplyAppearanceSeed(Seed);
        TestEqual(TEXT("Fresh recipe head stable"), A->HeadVariant, B->HeadVariant);
        TestEqual(TEXT("Fresh recipe hair stable"), A->HairVariant, B->HairVariant);
        TestEqual(TEXT("Fresh recipe garment stable"), A->GarmentVariant, B->GarmentVariant);
        TestEqual(TEXT("Fresh recipe accessory stable"), A->AccessoryVariant, B->AccessoryVariant);
        TestTrue(TEXT("Fresh recipe palette stable"),
                 A->SkinColor == B->SkinColor && A->HairColor == B->HairColor);
        SeedProof += FString::Printf(
            TEXT("%s head=%d hair=%d robe=%d accessory=%d skin=%s hairColor=%s\n"), Seed,
            A->HeadVariant, A->HairVariant, A->GarmentVariant, A->AccessoryVariant,
            *A->SkinColor.ToString(), *A->HairColor.ToString());
    }
    FString ProofPath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Visual11SeedProof.txt"));
    FParse::Value(FCommandLine::Get(), TEXT("VisualSeedEvidence="), ProofPath);
    TestTrue(TEXT("Seed proof recorded for process comparison"),
             FFileHelper::SaveStringToFile(SeedProof, *ProofPath));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVisual11PIE, "DaoVoTan.Visual11.PresentationPIE",
                                 EAutomationTestFlags::EditorContext |
                                     EAutomationTestFlags::EngineFilter)
class FVisual11Exercise : public IAutomationLatentCommand
{
    FAutomationTestBase *Test;
    int32 Stage = 0, Frames = 0, Samples = 0;
    double Since = 0, Started = FPlatformTime::Seconds(), Sum = 0, Game = 0, Render = 0, GPU = 0;
    FString Dir()
    {
        return FPaths::Combine(FPaths::ProjectSavedDir(),
                               FParse::Param(FCommandLine::Get(), TEXT("GateStress"))
                                   ? TEXT("Visual11Evidence-Stress")
                                   : TEXT("Visual11Evidence"));
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

  public:
    explicit FVisual11Exercise(FAutomationTestBase *T) : Test(T)
    {
    }
    bool Update() override
    {
        UWorld *W = nullptr;
        for (const auto &Context : GEngine->GetWorldContexts())
            if (Context.WorldType == EWorldType::PIE)
                W = Context.World();
        if (FPlatformTime::Seconds() - Started > 160)
        {
            Test->AddError(TEXT("Visual11 PIE timed out"));
            return true;
        }
        if (!W || !W->GetFirstPlayerController())
            return false;
        auto *PC = W->GetFirstPlayerController();
        auto *C = Cast<AGateCharacter>(PC->GetPawn());
        if (!C)
            return false;
        double T = FPlatformTime::Seconds() - Since;
        switch (Stage)
        {
        case 0:
            PC->SetIgnoreLookInput(true);
            GEngine->SetEngineStat(W, W->GetGameViewport(), TEXT("Unit"), false);
            C->SetActorLocation(FVector(-12500, 0, 100));
            PC->SetControlRotation(FRotator(-8, 155, 0));
            for (TActorIterator<AGateCharacter> I(W); I; ++I)
            {
                Test->TestNotNull(TEXT("Recipe garment component"), I->Appearance->Garment.Get());
                Test->TestTrue(TEXT("Head hair accessory modules"),
                               I->Appearance->ModuleCount() >= 3);
                Test->TestTrue(TEXT("Garment follows body pose"),
                               I->Appearance->Garment->LeaderPoseComponent.Get() == I->GetMesh());
                I->Actions->ResetToLocomotionPresentation();
            }
            Next();
            break;
        case 1:
            if (T > 3)
            {
                Shot(TEXT("01-player-hero.png"));
                Next();
            }
            break;
        case 2:
            if (T > 1)
            {
                PC->SetControlRotation(FRotator(-12, 0, 0));
                C->Boom->TargetArmLength = 520;
                Shot(TEXT("02-player-npcs.png"));
                Next();
            }
            break;
        case 3:
            if (T > 1)
            {
                C->GetMesh()->PlayAnimation(
                    LoadObject<UAnimSequence>(nullptr,
                                              TEXT("/Game/DaoVoTan/Visual11/AS_SwordIdle")),
                    true);
                Shot(TEXT("03-sword-idle.png"));
                Next();
            }
            break;
        case 4:
            if (T > .7)
            {
                C->Actions->PlaySwordPresentation();
                Next();
            }
            break;
        case 5:
            if (T > .42)
            {
                Shot(TEXT("04-sword-attack.png"));
                Next();
            }
            break;
        case 6:
            if (T > 1.5)
            {
                C->Actions->StartMeditationPresentation();
                Next();
            }
            break;
        case 7:
            if (T > .5)
            {
                Shot(TEXT("05-meditation-enter.png"));
                Next();
            }
            break;
        case 8:
            if (T > 1.5)
            {
                Test->TestTrue(TEXT("Niagara activated"),
                               C->Aura->NiagaraInstance() &&
                                   C->Aura->NiagaraInstance()->IsActive());
                Shot(TEXT("06-aura-calm.png"));
                C->CycleAura();
                Next();
            }
            break;
        case 9:
            if (T > 1.5)
            {
                Shot(TEXT("07-aura-dense.png"));
                C->CycleAura();
                Next();
            }
            break;
        case 10:
            if (T > 1.5)
            {
                Shot(TEXT("08-aura-high-energy.png"));
                C->Actions->StopMeditationPresentation();
                Next();
            }
            break;
        case 11:
            if (T > 2)
            {
                int32 I = 0;
                for (TActorIterator<AGateCharacter> A(W); A && I < 3; ++A, ++I)
                {
                    FBufferArchive PNG;
                    auto *Target = A->Portrait->RequestPortrait();
                    Test->TestTrue(TEXT("Portrait exported from actual actor"),
                                   FImageUtils::ExportRenderTarget2DAsPNG(Target, PNG));
                    FFileHelper::SaveArrayToFile(
                        PNG, *FPaths::Combine(Dir(), FString::Printf(TEXT("portrait-%d.png"), I)));
                }
                Shot(TEXT("09-portrait-ui.png"));
                Next();
            }
            break;
        case 12:
            if (T > 1)
            {
                C->Boom->TargetArmLength = 720;
                PC->SetControlRotation(FRotator(-16, 0, 0));
                Shot(TEXT("10-valley.png"));
                for (TActorIterator<AGateCharacter> I(W); I; ++I)
                    I->Aura->SetEmitting(true);
                GEngine->SetEngineStat(W, W->GetGameViewport(), TEXT("Unit"), true);
                Next();
            }
            break;
        case 13:
            if (T < 3)
                break;
            ++Frames;
            Sum += FApp::GetDeltaTime() * 1000;
            if (auto *Data = W->GetGameViewport()->GetStatUnitData())
            {
                Game += Data->GameThreadTime;
                Render += Data->RenderThreadTime;
                GPU += Data->GPUFrameTime[0];
                ++Samples;
            }
            if (T > 11)
            {
                int32 Actors = 0, Visible = 0, Niagara = 0;
                for (TActorIterator<AGateCharacter> I(W); I; ++I)
                {
                    ++Actors;
                    if (I->Appearance->Garment->WasRecentlyRendered(.3f))
                        ++Visible;
                    if (I->Aura->NiagaraInstance()->IsActive())
                        ++Niagara;
                }
                FString Result = FString::Printf(
                    TEXT("mode=EditorPIE\nresolution=%dx%d\nframes=%d\nfps=%.3f\nframe_ms=%."
                         "3f\ngame_thread_ms=%.3f\nrender_thread_ms=%.3f\ngpu_ms=%.3f\ncharacter_"
                         "actors=%d\nvisible_garments=%d\nNiagara_instances=%d\nshared_Niagara_"
                         "assets=1\nportrait_capture_limit=3\n"),
                    W->GetGameViewport()->Viewport->GetSizeXY().X,
                    W->GetGameViewport()->Viewport->GetSizeXY().Y, Frames, Frames * 1000 / Sum,
                    Sum / Frames, Game / FMath::Max(Samples, 1), Render / FMath::Max(Samples, 1),
                    GPU / FMath::Max(Samples, 1), Actors, Visible, Niagara);
                FFileHelper::SaveStringToFile(Result,
                                              *FPaths::Combine(Dir(), TEXT("performance.txt")));
                Test->AddInfo(Result);
                Next();
            }
            break;
        default:
            return T > 1;
        }
        return false;
    }
};
bool FVisual11PIE::RunTest(const FString &Params)
{
    ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/DaoVoTan/Maps/StillwaterValley")));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitForShadersToFinishCompiling());
    ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
    ADD_LATENT_AUTOMATION_COMMAND(FVisual11Exercise(this));
    ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
    return true;
}
#endif
