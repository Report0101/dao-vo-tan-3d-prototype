#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "AppearanceComponent.h"
#include "GateCharacter.h"
#include "GatePlayerController.h"
#include "CultivationCommandAdapter.h"
#include "SnapshotPresentationAdapter.h"
#include "CultivatorLocomotion.h"
#include "PresentationActions.h"
#include "PresentationRecipe.h"
#include "PresentationEvidence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "Rendering/SkeletalMeshLODRenderData.h"
#include "Engine/LocalPlayer.h"
#include "EngineUtils.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputKeyEventArgs.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"

namespace
{
const TCHAR *PreviewPath = TEXT("/Game/DaoVoTan/Experimental/PlayerV1/DA_PlayerV1Experimental");
FString PreviewDir()
{
    return FPaths::ProjectSavedDir() / (FParse::Param(FCommandLine::Get(), TEXT("PreviewInspectionCamera"))
        ? TEXT("PlayerV1PreviewWideEvidence") : TEXT("PlayerV1PreviewEvidence"));
}
bool SavePreviewJson(const TSharedRef<FJsonObject> &Object, const FString &Name)
{
    FString Text;
    auto Writer = TJsonWriterFactory<>::Create(&Text);
    FJsonSerializer::Serialize(Object, Writer);
    IFileManager::Get().MakeDirectory(*PreviewDir(), true);
    return FFileHelper::SaveStringToFile(Text, *(PreviewDir() / Name));
}
void MeshCost(USkeletalMesh *Mesh, int32 &Sections, int32 &Triangles)
{
    if (auto *Render = Mesh ? Mesh->GetResourceForRendering() : nullptr)
        if (Render->LODRenderData.Num())
            for (const auto &S : Render->LODRenderData[0].RenderSections)
            { ++Sections; Triangles += S.NumTriangles; }
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPlayerV1PreviewAssets, "DaoVoTan.Preview.AssetContract",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPlayerV1PreviewAssets::RunTest(const FString &)
{
    auto *Recipe = LoadObject<UPresentationRecipe>(nullptr, PreviewPath);
    auto *Default = LoadObject<UPresentationRecipe>(nullptr, TEXT("/Game/DaoVoTan/Data/DA_Player"));
    auto *Manny = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple"));
    if (!TestNotNull(TEXT("Experimental recipe exists"), Recipe) ||
        !TestNotNull(TEXT("Accepted default recipe exists"), Default) ||
        !TestNotNull(TEXT("Original Manny leader exists"), Manny)) return false;
    TestTrue(TEXT("Accepted recipe still uses its original variant appearance"), Default->SkeletalModules.IsEmpty());
    TestEqual(TEXT("Seven explicit preview material bindings"), Recipe->SkeletalSlotMaterials.Num(), 7);
    TestTrue(TEXT("Default recipe material bindings unchanged"), Default->SkeletalSlotMaterials.IsEmpty());
    TestEqual(TEXT("Exactly 15 experimental recipe modules"), Recipe->SkeletalModules.Num(), 15);
    int32 Sections = 0, Triangles = 0;
    TSet<USkeletalMesh *> Unique;
    for (const auto &Ref : Recipe->SkeletalModules)
    {
        auto *M = Ref.LoadSynchronous();
        if (!TestNotNull(TEXT("Recipe module resolves"), M)) continue;
        Unique.Add(M);
        for (const auto &Slot : M->GetMaterials())
        {
            const auto *Material = Recipe->SkeletalSlotMaterials.Find(Slot.MaterialSlotName);
            TestTrue(TEXT("Every candidate slot has a loadable preview binding"), Material && Material->LoadSynchronous());
        }
        TestTrue(TEXT("Candidate remains within Gate 1B boundary"), M->GetPathName().StartsWith(
            TEXT("/Game/DaoVoTan/Experimental/PlayerV1/RetopoGate1/")));
        TestTrue(TEXT("Original Manny skeleton"), M->GetSkeleton() == Manny->GetSkeleton());
        TestEqual(TEXT("89 reference bones"), M->GetRefSkeleton().GetNum(), 89);
        MeshCost(M, Sections, Triangles);
    }
    TestEqual(TEXT("No duplicate modules"), Unique.Num(), 15);
    TestEqual(TEXT("Unmodified candidate section count"), Sections, 18);
    TestEqual(TEXT("Unmodified candidate triangle count"), Triangles, 46248);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPlayerV1PreviewLive, "DaoVoTan.Preview.LivePIE",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
struct FPlayerV1PreviewProof
{
    TSet<FString> Sessions;
    int64 WorldTick = -1;
    FString Trace;
};
class FPlayerV1PreviewExercise : public IAutomationLatentCommand
{
    FAutomationTestBase *Test;
    TSharedRef<FPlayerV1PreviewProof> Proof;
    int32 Session, Stage = 0, Command = 0, OriginalInstances = 0, SwordBefore = 0;
    double Started = FPlatformTime::Seconds(), Since = 0;
    double PreviousWall=FPlatformTime::Seconds(), WallFrameMs=0;
    FKey Release;
    TWeakObjectPtr<AGateCharacter> Pawn;
    TWeakObjectPtr<UAnimInstance> Animation;
    TWeakObjectPtr<UCharacterMovementComponent> Movement;
    TWeakObjectPtr<AGatePlayerController> Controller;
    TWeakObjectPtr<UPresentationRecipe> CurrentRecipe;
    FString ActorId, SessionId, RequestId, RequestJson;
    FTransform Grip;
    FVector StartPosition;
    int64 BeforeTick = 0;
    int32 BeforeResults = 0, BeforeCompletions = 0, Days = 1;
    bool SawLanding = false, MeditationCaptured = false;
    uint32 ActionRevision = 0;
    TArray<double> CurrentMs, PreviewMs;
    void Next(UWorld *W) { ++Stage; Since = W->GetTimeSeconds(); }
    void Pulse(AGatePlayerController *PC, FKey Key)
    {
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, IE_Pressed, 1.f));
        Release = Key;
    }
    void Inject(AGateCharacter *C, FKey Key)
    {
        auto *PC = Cast<APlayerController>(C->GetController());
        auto *Sub = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
        for (const auto &M : C->GetPresentationInputContext()->GetMappings())
            if (M.Key == Key)
            {
                TArray<UInputModifier *> Mods;
                for (auto Mod : M.Modifiers) Mods.Add(Mod.Get());
                Sub->InjectInputForAction(M.Action, FInputActionValue(1.f), Mods, {});
                break;
            }
    }
    void Shot(AGateCharacter *C, const TCHAR *Name)
    {
        auto *Anim = Cast<UCultivatorLocomotion>(C->GetMesh()->GetAnimInstance());
        Proof->Trace += FString::Printf(TEXT("shot=%s session=%d preview=%d pawn=%s actor=%s db=%s clip=%s speed=%.2f phase=%d\n"),
            Name, Session, C->Appearance->IsExperimentalPreview(), *C->GetName(), *ActorId,
            Anim ? *Anim->SelectedDatabase : TEXT("none"), Anim ? *Anim->SelectedAnimation : TEXT("none"),
            C->GetVelocity().Size2D(), int32(C->Actions->GetPhase()));
        Test->TestTrue(TEXT("Live PIE backbuffer screenshot"), CapturePIEEvidence(
            PreviewDir() / FString::Printf(TEXT("session%d-%s.png"), Session + 1, Name)));
    }
    void Identity(AGateCharacter *C, AGatePlayerController *PC, USnapshotPresentationAdapter *A)
    {
        Test->TestTrue(TEXT("Same possessed pawn"), C == Pawn.Get() && PC->GetPawn() == Pawn.Get());
        Test->TestTrue(TEXT("Same controller and movement component"), PC == Controller.Get() && C->GetCharacterMovement() == Movement.Get());
        Test->TestTrue(TEXT("Same animation instance and locomotion stack"), C->GetMesh()->GetAnimInstance() == Animation.Get());
        Test->TestTrue(TEXT("Current actor recipe unchanged by visual toggle"), C->Recipe == CurrentRecipe.Get());
        Test->TestTrue(TEXT("Single appearance owner retains current recipe"), C->Appearance->Recipe == C->Recipe);
        Test->TestEqual(TEXT("Simulation actor identity retained"), A->Snapshot().ControlledActorId, ActorId);
        Test->TestEqual(TEXT("Controller session identity retained"), PC->CultivationCommands->PresentationSessionId, SessionId);
        Test->TestTrue(TEXT("Unchanged accepted sword grip transform"), C->WeaponSocket->GetRelativeTransform().Equals(Grip));
        Test->TestTrue(TEXT("Sword stays on original Manny leader"), C->WeaponSocket->GetAttachParent() == C->GetMesh());
    }
    void Cost(AGateCharacter *C)
    {
        int32 Count = 0, Sections = 0, Triangles = 0, RecentlyRendered = 0;
        for (auto M : C->Appearance->GetSkinnedModules())
        {
            Test->TestTrue(TEXT("Visible registered follower"), M && M->IsRegistered() && M->IsVisible());
            Test->TestTrue(TEXT("Same Manny leader pose; no extra animation instance"),
                M && M->LeaderPoseComponent.Get() == C->GetMesh() && !M->GetAnimInstance());
            if (!M) continue;
            auto *Profile = LoadObject<UPresentationRecipe>(nullptr, PreviewPath);
            for (int32 I=0;I<M->GetSkeletalMeshAsset()->GetMaterials().Num();++I)
            {
                const auto *Material = Profile->SkeletalSlotMaterials.Find(M->GetSkeletalMeshAsset()->GetMaterials()[I].MaterialSlotName);
                Test->TestTrue(TEXT("Actual live component uses recipe material override"), Material && M->GetMaterial(I)==Material->LoadSynchronous());
            }
            ++Count;
            RecentlyRendered += M->WasRecentlyRendered(.3f) ? 1 : 0;
            MeshCost(M->GetSkeletalMeshAsset(), Sections, Triangles);
        }
        Test->TestEqual(TEXT("15 live modular components"), Count, 15);
        Test->TestEqual(TEXT("18 live skeletal sections"), Sections, 18);
        Test->TestEqual(TEXT("46248 live candidate triangles"), Triangles, 46248);
        auto Report = MakeShared<FJsonObject>();
        const FIntPoint ViewportSize = C->GetWorld()->GetGameViewport()->Viewport->GetSizeXY();
        Report->SetNumberField(TEXT("viewportWidth"), ViewportSize.X);
        Report->SetNumberField(TEXT("viewportHeight"), ViewportSize.Y);
        Report->SetNumberField(TEXT("visibleRegisteredModules"), Count);
        Report->SetNumberField(TEXT("recentlyRenderedModules"), RecentlyRendered);
        Report->SetNumberField(TEXT("skeletalSectionsLOD0"), Sections);
        Report->SetNumberField(TEXT("trianglesLOD0"), Triangles);
        for (const auto &Item : {TPair<FString,TArray<double>>(TEXT("current"), CurrentMs),
                                  TPair<FString,TArray<double>>(TEXT("experimental"), PreviewMs)})
        {
            auto Samples = Item.Value; Samples.Sort();
            if (Samples.IsEmpty()) continue;
            double Sum = 0; for (double V : Samples) Sum += V;
            auto Stats = MakeShared<FJsonObject>();
            Stats->SetNumberField(TEXT("samples"), Samples.Num());
            Stats->SetNumberField(TEXT("meanFrameMs"), Sum / Samples.Num());
            Stats->SetNumberField(TEXT("meanFPS"), 1000. / (Sum / Samples.Num()));
            Stats->SetNumberField(TEXT("medianFrameMs"), Samples[Samples.Num()/2]);
            Stats->SetNumberField(TEXT("p95FrameMs"), Samples[FMath::Min(Samples.Num()-1, FMath::FloorToInt(Samples.Num()*.95))]);
            Report->SetObjectField(Item.Key, Stats);
        }
        Report->SetStringField(TEXT("setup"), TEXT("Same live PIE scene/pawn/location/view, test-only 550cm camera distance; wall interval between PIE automation updates, editor/worker overhead included; no GPU timing. Short sanity sample, not a benchmark."));
        Test->TestTrue(TEXT("Performance sanity report"), SavePreviewJson(Report, TEXT("performance.json")));
    }
    void BeginCultivate(AGateCharacter *C, AGatePlayerController *PC, USnapshotPresentationAdapter *A)
    {
        Days = Session == 0 ? (Command == 0 ? 1 : 7) : Session == 1 ? 17 : 30;
        BeforeTick = A->Snapshot().WorldTick;
        BeforeResults = PC->CultivationCommands->LogicalResultCount;
        BeforeCompletions = C->Actions->MeditationCompletionCount;
        PC->CultivationCommands->SelectCultivationDays(Days);
        Pulse(PC, Session == 2 || Command == 1 ? EKeys::M : EKeys::E);
        MeditationCaptured = false;
    }
  public:
    FPlayerV1PreviewExercise(FAutomationTestBase *T, TSharedRef<FPlayerV1PreviewProof> P, int32 S)
        : Test(T), Proof(P), Session(S) {}
    bool Update() override
    {
        const double NowWall=FPlatformTime::Seconds(); WallFrameMs=(NowWall-PreviousWall)*1000.; PreviousWall=NowWall;
        if (NowWall - Started > 180)
        { Test->AddError(FString::Printf(TEXT("Player V1 preview timeout stage=%d session=%d"),Stage,Session)); return true; }
        UWorld *W = nullptr;
        for (const auto &X : GEngine->GetWorldContexts()) if (X.WorldType == EWorldType::PIE) W = X.World();
        auto *PC = W ? Cast<AGatePlayerController>(W->GetFirstPlayerController()) : nullptr;
        auto *C = PC ? Cast<AGateCharacter>(PC->GetPawn()) : nullptr;
        auto *A = W && W->GetAuthGameMode() ? W->GetAuthGameMode()->FindComponentByClass<USnapshotPresentationAdapter>() : nullptr;
        if (!C || !A) return false;
        auto *Commands = PC->CultivationCommands.Get();
        auto *Anim = Cast<UCultivatorLocomotion>(C->GetMesh()->GetAnimInstance());
        if (!Anim) return false;
        if (Release.IsValid())
        { PC->InputKey(FInputKeyEventArgs::CreateSimulated(Release, IE_Released, 0.f)); Release=FKey(); }
        if (!C->Actions->IsMeditating() || FParse::Param(FCommandLine::Get(),TEXT("PreviewInspectionCamera"))) C->Boom->TargetArmLength = 550; // Fixture framing only.
        if (FParse::Param(FCommandLine::Get(),TEXT("PreviewInspectionCamera")))
            C->Boom->SocketOffset.Z = C->Actions->IsMeditating() ? -50.f : 58.f; // Observer framing above HUD only.
        const double T = W->GetTimeSeconds() - Since;
        if (Anim->bLanding || Anim->SelectedDatabase.Contains(TEXT("Landing"))) SawLanding = true;
        switch (Stage)
        {
        case 0:
            if (W->GetTimeSeconds() > 3 && Commands->IsConnected() &&
                !A->Snapshot().ControlledActorId.IsEmpty() && !Proof->Sessions.Contains(Commands->PresentationSessionId))
            {
                Test->TestFalse(TEXT("Every new PIE session defaults to accepted current player"), C->Appearance->IsExperimentalPreview());
                Test->TestTrue(TEXT("No experimental modules before opt-in"), C->Appearance->GetSkinnedModules().IsEmpty());
                Test->TestTrue(TEXT("Original accepted garment exists"), C->Appearance->Garment != nullptr);
                Pawn=C; Animation=Anim; Movement=C->GetCharacterMovement(); Controller=PC; CurrentRecipe=C->Recipe; ActorId=A->Snapshot().ControlledActorId;
                SessionId=Commands->PresentationSessionId; Proof->Sessions.Add(SessionId); Grip=C->WeaponSocket->GetRelativeTransform();
                OriginalInstances=C->GetInstanceComponents().Num();
                if(Proof->WorldTick>=0) Test->TestEqual(TEXT("World survives Stop/Play"), A->Snapshot().WorldTick, Proof->WorldTick);
                C->SetActorLocation(FVector(-11500,0,110)); PC->SetControlRotation(FRotator(-12,0,0));
                C->Boom->TargetArmLength=550; StartPosition=C->GetActorLocation(); Next(W);
            }
            break;
        case 1:
            if (T>.4 && Session==0) CurrentMs.Add(WallFrameMs);
            if (T>1.5) { Shot(C,TEXT("current-player")); Pulse(PC,EKeys::F7); Next(W); }
            break;
        case 2:
            if(T>.5)
            {
                Test->TestTrue(TEXT("Real F7 input enables experimental appearance"),C->Appearance->IsExperimentalPreview());
                Identity(C,PC,A);
                for(TActorIterator<AGateCharacter> I(W);I;++I)
                    if(*I!=C) Test->TestFalse(TEXT("NPC appearance not toggled"),I->Appearance->IsExperimentalPreview());
                Next(W);
            }
            break;
        case 3:
            if(Session==0 && T>.4) PreviewMs.Add(WallFrameMs);
            if(T>1.5)
            {
                if(Session==0) Cost(C);
                Shot(C,TEXT("experimental-idle-back"));
                if(Session>0) { Stage=18; Since=W->GetTimeSeconds(); BeginCultivate(C,PC,A); }
                else { PC->SetControlRotation(FRotator(-12,180,0)); Next(W); }
            }
            break;
        case 4:
            if(T>.7) { Shot(C,TEXT("front")); PC->SetControlRotation(FRotator(-12,135,0)); Stage=24; Since=W->GetTimeSeconds(); }
            break;
        case 24:
            if(T>.7) { Shot(C,TEXT("three-quarter")); PC->SetControlRotation(FRotator(-12,90,0)); Stage=5; Since=W->GetTimeSeconds(); }
            break;
        case 5:
            if(T>.7) { Shot(C,TEXT("side")); PC->SetControlRotation(FRotator(-12,0,0)); Next(W); }
            break;
        case 6:
            Inject(C,EKeys::W);
            if(T>1.3) { Test->TestTrue(TEXT("Preview walks through existing movement"),C->GetVelocity().Size2D()>100); Shot(C,TEXT("walk")); Pulse(PC,EKeys::LeftShift); Next(W); }
            break;
        case 7:
            Inject(C,EKeys::W);
            if(T>1.2) { Test->TestTrue(TEXT("Preview runs through Motion Matching"),Anim->SelectedAnimation.Contains(TEXT("_Run_"))); Shot(C,TEXT("run")); Next(W); }
            break;
        case 8:
            Inject(C,EKeys::S);
            if(T>.3) { Shot(C,TEXT("pivot")); Pulse(PC,EKeys::LeftShift); Next(W); }
            break;
        case 9:
            if(T>.8) { PC->SetControlRotation(FRotator(-12,C->GetActorRotation().Yaw,0)); Pulse(PC,EKeys::Q); Next(W); }
            break;
        case 10:
            Inject(C,EKeys::A);
            if(T>1) { Test->TestTrue(TEXT("Preview uses existing strafing mode"),C->IsStrafing()); Shot(C,TEXT("strafe")); Next(W); }
            break;
        case 11:
            if(T>.8) { Pulse(PC,EKeys::Q); Next(W); }
            break;
        case 12:
            if(T>.2) { SawLanding=false; Pulse(PC,EKeys::SpaceBar); Next(W); }
            break;
        case 13:
            if(T>.2 && C->GetCharacterMovement()->IsFalling()) { Shot(C,TEXT("jump")); Next(W); }
            break;
        case 14:
            if(T>1 && !C->GetCharacterMovement()->IsFalling())
            {
                Test->TestTrue(TEXT("Landing state observed in same animation stack"),SawLanding);
                Shot(C,TEXT("landing")); PC->SetControlRotation(FRotator(-12,C->GetActorRotation().Yaw+145,0)); SwordBefore=C->Actions->SwordStartCount; Pulse(PC,EKeys::LeftMouseButton); Next(W);
            }
            break;
        case 15:
            if(T>.25)
            {
                Test->TestEqual(TEXT("LMB starts accepted sword action"),C->Actions->SwordStartCount,SwordBefore+1);
                Test->TestTrue(TEXT("Sword clip advances in original anim instance"),Anim->ActionSequenceTime>.1);
                Shot(C,TEXT("sword-attack")); ActionRevision=Anim->ActionRevision; Pulse(PC,EKeys::F7); Next(W);
            }
            break;
        case 16:
            if(T>.15)
            {
                Test->TestFalse(TEXT("Toggle back works during sword"),C->Appearance->IsExperimentalPreview());
                Test->TestEqual(TEXT("Toggle does not restart sword animation"),Anim->ActionRevision,ActionRevision);
                Identity(C,PC,A); Pulse(PC,EKeys::F7); Next(W);
            }
            break;
        case 17:
            if(T>1.8 && C->Actions->GetPhase()==EPresentationPhase::Locomotion)
            {
                Test->TestTrue(TEXT("Experimental restored without action reset"),C->Appearance->IsExperimentalPreview());
                Test->TestTrue(TEXT("Existing sword carry/guard layer retained"),Anim->bHasSword && Anim->CarryClip);
                Shot(C,TEXT("sword-guard")); BeginCultivate(C,PC,A); Next(W);
            }
            break;
        case 18:
            if(!Commands->LastRequestId.IsEmpty() && C->Actions->IsMeditating())
            {
                RequestId=Commands->LastRequestId; RequestJson=Commands->LastRequestJson;
                Test->TestEqual(TEXT("Command remains bound to controlled actor"),Commands->ExpectedActor,ActorId);
                Pulse(PC,EKeys::F7); Next(W);
            }
            break;
        case 19:
            if(T>.2)
            {
                Test->TestFalse(TEXT("Current appearance during authoritative meditation"),C->Appearance->IsExperimentalPreview());
                Test->TestEqual(TEXT("Request identity unchanged by toggle"),Commands->LastRequestId,RequestId);
                Test->TestEqual(TEXT("Request payload unchanged by toggle"),Commands->LastRequestJson,RequestJson);
                Test->TestTrue(TEXT("Toggle does not stop meditation"),C->Actions->IsMeditating());
                Identity(C,PC,A); Pulse(PC,EKeys::F7); Next(W);
            }
            break;
        case 20:
            if(C->Actions->GetPhase()==EPresentationPhase::MeditationLoop && !MeditationCaptured)
            { Test->TestTrue(TEXT("Preview active during meditation"),C->Appearance->IsExperimentalPreview()); Shot(C,TEXT("meditation")); MeditationCaptured=true; }
            if(Commands->Receipt.Status==TEXT("completed") && C->Actions->GetPhase()==EPresentationPhase::Locomotion && T>3)
            {
                Test->TestTrue(TEXT("Live meditation loop captured"),MeditationCaptured);
                Test->TestEqual(TEXT("Authoritative completion once"),Commands->LogicalResultCount,BeforeResults+1);
                Test->TestEqual(TEXT("Meditation presentation completion once"),C->Actions->MeditationCompletionCount,BeforeCompletions+1);
                Test->TestEqual(TEXT("Expected simulation progress"),A->Snapshot().WorldTick,BeforeTick+Days);
                Test->TestEqual(TEXT("Final receipt request remains unchanged"),Commands->Receipt.RequestId,RequestId);
                Proof->Trace+=FString::Printf(TEXT("cultivate session=%d actor=%s request=%s action=%s days=%d before=%lld after=%lld\n"),Session,*ActorId,*RequestId,*Commands->Receipt.ActionId,Days,BeforeTick,A->Snapshot().WorldTick);
                if(Session==0 && Command==0) { ++Command; Stage=23; Since=W->GetTimeSeconds(); }
                else { Pulse(PC,EKeys::F7); Next(W); }
            }
            break;
        case 21:
            if(T>.6)
            {
                Test->TestFalse(TEXT("Human can return to current player without restart"),C->Appearance->IsExperimentalPreview());
                Test->TestTrue(TEXT("Accepted garment restored"),C->Appearance->Garment!=nullptr && C->Appearance->GetSkinnedModules().IsEmpty());
                Test->TestEqual(TEXT("No instance-component leak after repeated switching"),C->GetInstanceComponents().Num(),OriginalInstances);
                Identity(C,PC,A); Shot(C,TEXT("current-restored")); Pulse(PC,EKeys::F7); Next(W);
            }
            break;
        case 23:
            if(T>.35) { BeginCultivate(C,PC,A); Stage=18; Since=W->GetTimeSeconds(); }
            break;
        case 22:
            if(T>.5)
            {
                Test->TestTrue(TEXT("Stop PIE while preview selected"),C->Appearance->IsExperimentalPreview());
                Identity(C,PC,A); Proof->WorldTick=A->Snapshot().WorldTick;
                Test->TestTrue(TEXT("Preview does not prevent movement travel"),Session>0 || FVector::Dist2D(StartPosition,C->GetActorLocation())>150);
                Test->TestTrue(TEXT("Live trace saved"),FFileHelper::SaveStringToFile(Proof->Trace,*(PreviewDir()/TEXT("live-proof.txt"))));
                return true;
            }
            break;
        }
        return false;
    }
};
bool FPlayerV1PreviewLive::RunTest(const FString &)
{
    if(!FParse::Param(FCommandLine::Get(),TEXT("PlayerV1PreviewTest")))
    { AddInfo(TEXT("Dedicated real-worker -PlayerV1PreviewTest fixture required")); return true; }
    auto Proof=MakeShared<FPlayerV1PreviewProof>();
    ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/DaoVoTan/Maps/StillwaterValley")));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitForShadersToFinishCompiling());
    const int32 SessionCount=FParse::Param(FCommandLine::Get(),TEXT("PreviewInspectionCamera")) ? 1 : 3;
    for(int32 S=0;S<SessionCount;++S)
    {
        ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
        ADD_LATENT_AUTOMATION_COMMAND(FPlayerV1PreviewExercise(this,Proof,S));
        ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
    }
    return true;
}
#endif
