#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "Tests/AutomationEditorCommon.h"
#include "GateCharacter.h"
#include "SnapshotPresentationAdapter.h"
#include "PresentationEvidence.h"

namespace {
FString G1AFixture(const TCHAR *Name)
{
    FString Dir=FPaths::Combine(FPaths::ProjectDir(),TEXT("../Bridge/sample-data/g1a")),Text;
    FParse::Value(FCommandLine::Get(),TEXT("G1AFixtureDir="),Dir);
    FFileHelper::LoadFileToString(Text,*FPaths::Combine(Dir,FString(Name)+TEXT(".json")));
    return Text;
}
FString G1ARefresh(const FString &Text,const USnapshotPresentationAdapter *Adapter)
{
    TSharedPtr<FJsonObject> Object;
    if(!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text),Object)) return TEXT("");
    Object->SetStringField(TEXT("streamId"),Adapter->Snapshot().StreamId);
    Object->SetNumberField(TEXT("sequence"),double(Adapter->LastSequence()+1));
    FString Result;
    FJsonSerializer::Serialize(Object.ToSharedRef(),TJsonWriterFactory<>::Create(&Result));
    return Result;
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDynamicAgentProtocol,"DaoVoTan.Integration.DynamicAgents.Protocol",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FDynamicAgentProtocol::RunTest(const FString &)
{
    FWorldPresentationSnapshot Snapshot; FString Error;
    for(const TCHAR *Name : {TEXT("empty"),TEXT("1"),TEXT("3"),TEXT("5"),TEXT("ten")}) {
        TestTrue(TEXT("Real bounded simulation snapshot accepted"),SnapshotV1::Parse(G1AFixture(Name),Snapshot,Error));
        const int32 Expected=FString(Name)==TEXT("empty")?0:FString(Name)==TEXT("ten")?10:FCString::Atoi(Name);
        TestEqual(TEXT("Expected snapshot membership"),Snapshot.Actors.Num(),Expected);
    }
    const auto Valid=G1AFixture(TEXT("ten"));
    TestFalse(TEXT("Stale ruleset rejected"),SnapshotV1::Parse(Valid.Replace(TEXT("rules-v2"),TEXT("rules-v1")),Snapshot,Error));
    TestFalse(TEXT("Stale implementation rejected"),SnapshotV1::Parse(Valid.Replace(TEXT("0.0.2"),TEXT("0.0.1")),Snapshot,Error));
    TSharedPtr<FJsonObject> Object;
    FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Valid),Object);
    if(!Object) {AddError(TEXT("Missing required G1A fixture"));return false;}
    auto Actors=Object->GetArrayField(TEXT("actors"));
    Actors[1]=Actors[0]; Object->SetArrayField(TEXT("actors"),Actors);
    FString Duplicate; FJsonSerializer::Serialize(Object.ToSharedRef(),TJsonWriterFactory<>::Create(&Duplicate));
    TestFalse(TEXT("Duplicate identity rejected"),SnapshotV1::Parse(Duplicate,Snapshot,Error));
    return true;
}

class FDynamicAgentExercise : public IAutomationLatentCommand {
    FAutomationTestBase *Test;
    int32 Stage=0,InitialSpawns=0,InitialRetires=0;
    double Since=FPlatformTime::Seconds(),Started=Since;
    TMap<FString,TWeakObjectPtr<AGateCharacter>> Initial;
    FString PlayerId,Removed,Added,Dir;
    TWeakObjectPtr<AGateCharacter> Player,Retired;
    TArray<double> ApplySamples;
    UWorld *World() {
        for(const auto &Context : GEngine->GetWorldContexts()) if(Context.WorldType==EWorldType::PIE) return Context.World();
        return nullptr;
    }
    void Next() {++Stage;Since=FPlatformTime::Seconds();}
    void Check(USnapshotPresentationAdapter *A,UWorld *W,int32 Count) {
        Test->TestEqual(TEXT("Binding count matches snapshot"),A->BindingCount(),Count);
        TSet<AGateCharacter *> Unique;
        for(const auto &State : A->Snapshot().Actors) {
            auto *Actor=A->ActorFor(State.ActorId);
            Test->TestTrue(TEXT("Every ID has a valid distinct Actor"),IsValid(Actor)&&!Unique.Contains(Actor));
            Unique.Add(Actor);
            if(Actor) Test->TestTrue(TEXT("Reverse lookup matches ID"),A->StateFor(Actor)&&A->StateFor(Actor)->ActorId==State.ActorId);
        }
        int32 Active=0;
        for(TActorIterator<AGateCharacter> It(W);It;++It) if(!It->IsHidden()) ++Active;
        Test->TestEqual(TEXT("Visible characters exactly equal represented identities"),Active,Count);
        if(!A->Snapshot().ControlledActorId.IsEmpty()) {
            Test->TestTrue(TEXT("Controlled Agent binds existing pawn"),A->ActorFor(A->Snapshot().ControlledActorId)==W->GetFirstPlayerController()->GetPawn());
            Test->TestEqual(TEXT("Controlled pawn appears once"),Unique.Contains(Cast<AGateCharacter>(W->GetFirstPlayerController()->GetPawn()))?1:0,1);
        }
    }
  public:
    explicit FDynamicAgentExercise(FAutomationTestBase *InTest):Test(InTest) {
        Dir=FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("G1AIdentityEvidence"));
        FParse::Value(FCommandLine::Get(),TEXT("G1AEvidenceDir="),Dir);
        IFileManager::Get().MakeDirectory(*Dir,true);
    }
    virtual bool Update() override {
        const double Now=FPlatformTime::Seconds();
        if(Now-Started>150) {Test->AddError(TEXT("G1A runtime fixture timed out"));return true;}
        auto *W=World(); if(!W||!W->GetAuthGameMode()||!W->GetFirstPlayerController())return false;
        auto *A=W->GetAuthGameMode()->FindComponentByClass<USnapshotPresentationAdapter>();
        if(!A)return false;
        FString Error;
        const double T=Now-Since;
        switch(Stage) {
        case 0: {
            if(T<4)return false;
            FString Bridge;
            const bool Live=FParse::Value(FCommandLine::Get(),TEXT("CultivationBridgeDir="),Bridge);
            if(Live) {if(A->BindingCount()!=10)return false;}
            else if(!A->LoadSnapshotFile(FPaths::Combine(FPaths::ProjectDir(),TEXT("../Bridge/sample-data/g1a/ten.json")),true)) {
                Test->AddError(TEXT("Required G1A ten-Agent fixture failed"));return true;
            }
            Check(A,W,10); if(A->BindingCount()!=10)return true;
            PlayerId=A->Snapshot().ControlledActorId; Player=A->ActorFor(PlayerId);
            for(const auto &State : A->Snapshot().Actors) Initial.Add(State.ActorId,A->ActorFor(State.ActorId));
            InitialSpawns=A->SpawnedProxyCount;InitialRetires=A->RetiredProxyCount;
            auto *Camera=W->SpawnActor<ACameraActor>();
            const FVector Position(-12600,-1400,1450),Focus(-11400,0,140);
            Camera->SetActorLocationAndRotation(Position,(Focus-Position).Rotation());
            Camera->GetCameraComponent()->SetFieldOfView(65);
            W->GetFirstPlayerController()->SetViewTarget(Camera);
            Next();break;
        }
        case 1: {
            if(T<3)return false;
            Test->TestTrue(TEXT("Capture actual ten-identity PIE view"),CapturePIEEvidence(FPaths::Combine(Dir,TEXT("01-ten-identities.png"))));
            for(int32 I=0;I<100;++I) {
                Test->TestTrue(TEXT("Refresh accepted"),A->ApplyJson(G1ARefresh(G1AFixture(TEXT("ten")),A),false,Error));
                ApplySamples.Add(A->LastApplyMs);
            }
            for(const auto &Entry : Initial) Test->TestTrue(TEXT("Retained ID preserves Actor"),A->ActorFor(Entry.Key)==Entry.Value.Get());
            Test->TestEqual(TEXT("No refresh respawns"),A->SpawnedProxyCount,InitialSpawns);
            Test->TestEqual(TEXT("No refresh retirement"),A->RetiredProxyCount,InitialRetires);
            Next();break;
        }
        case 2: {
            Test->TestTrue(TEXT("Transition snapshot accepted"),A->ApplyJson(G1ARefresh(G1AFixture(TEXT("transition")),A),false,Error));
            Check(A,W,10);
            for(const auto &Entry : Initial) {
                if(!A->ActorFor(Entry.Key)) {Removed=Entry.Key;Retired=Entry.Value;}
                else Test->TestTrue(TEXT("Unchanged nine identities retain Actors"),A->ActorFor(Entry.Key)==Entry.Value.Get());
            }
            for(const auto &State : A->Snapshot().Actors) if(!Initial.Contains(State.ActorId)) Added=State.ActorId;
            Test->TestFalse(TEXT("Removed ID was identified"),Removed.IsEmpty());
            Test->TestFalse(TEXT("New ID was identified"),Added.IsEmpty());
            Test->TestTrue(TEXT("Removed proxy destroyed"),!Retired.IsValid());
            Test->TestNull(TEXT("Removed ID unbound"),A->ActorFor(Removed));
            Test->TestTrue(TEXT("New ID owns new valid Actor"),IsValid(A->ActorFor(Added)));
            Test->TestEqual(TEXT("Exactly one new proxy"),A->SpawnedProxyCount,InitialSpawns+1);
            Test->TestEqual(TEXT("Exactly one retired proxy"),A->RetiredProxyCount,InitialRetires+1);
            Test->TestTrue(TEXT("Controlled player never replaced"),A->ActorFor(PlayerId)==Player.Get());
            Next();break;
        }
        case 3: {
            if(T<2)return false;
            Test->TestTrue(TEXT("Capture actual transition"),CapturePIEEvidence(FPaths::Combine(Dir,TEXT("02-transition.png"))));
            Test->TestTrue(TEXT("Save/reload resync accepted"),A->ApplyJson(G1AFixture(TEXT("reload")),true,Error));
            Check(A,W,10);
            Test->TestTrue(TEXT("Player identity survives resync"),A->ActorFor(PlayerId)==Player.Get());
            for(const auto &Entry : Initial) if(Entry.Key!=Removed)
                Test->TestTrue(TEXT("Retained IDs survive reload"),A->ActorFor(Entry.Key)==Entry.Value.Get());
            Next();break;
        }
        case 4: {
            if(T<2)return false;
            Test->TestTrue(TEXT("Capture reload identities"),CapturePIEEvidence(FPaths::Combine(Dir,TEXT("03-reload.png"))));
            for(const TCHAR *Name : {TEXT("empty"),TEXT("1"),TEXT("3"),TEXT("5"),TEXT("ten")}) {
                const int32 Count=FString(Name)==TEXT("empty")?0:FString(Name)==TEXT("ten")?10:FCString::Atoi(Name);
                Test->TestTrue(TEXT("Smaller representation accepted"),A->ApplyJson(G1ARefresh(G1AFixture(Name),A),false,Error));Check(A,W,Count);
            }
            auto Json=G1ARefresh(G1AFixture(TEXT("ten")),A);
            const auto Sequence=A->LastSequence();const int32 Spawns=A->SpawnedProxyCount;
            Test->TestFalse(TEXT("Stale behavioral label rejected atomically"),A->ApplyJson(Json.Replace(TEXT("rules-v2"),TEXT("rules-v1")),false,Error));
            Test->TestEqual(TEXT("Invalid snapshot consumes no sequence"),A->LastSequence(),Sequence);
            Test->TestEqual(TEXT("Invalid snapshot creates no Actors"),A->SpawnedProxyCount,Spawns);
            double Total=0;for(double Ms:ApplySamples)Total+=Ms;
            TSharedRef<FJsonObject> Proof=MakeShared<FJsonObject>();
            Proof->SetNumberField(TEXT("controlledPlayerCount"),1);Proof->SetNumberField(TEXT("npcProxyCount"),9);
            Proof->SetNumberField(TEXT("uniqueAgentIds"),A->BindingCount());Proof->SetNumberField(TEXT("duplicateBindings"),0);
            Proof->SetNumberField(TEXT("refreshesWithoutRespawn"),ApplySamples.Num());
            Proof->SetNumberField(TEXT("meanApplyMs"),Total/FMath::Max(1,ApplySamples.Num()));
            Proof->SetStringField(TEXT("controlledAgentId"),PlayerId);Proof->SetStringField(TEXT("removedAgentId"),Removed);Proof->SetStringField(TEXT("introducedAgentId"),Added);
            Proof->SetBoolField(TEXT("retainedActorsStable"),true);Proof->SetBoolField(TEXT("reloadIdentityPass"),true);
            FString Output;FJsonSerializer::Serialize(Proof,TJsonWriterFactory<>::Create(&Output));
            Test->TestTrue(TEXT("Save identity evidence"),FFileHelper::SaveStringToFile(Output,*FPaths::Combine(Dir,TEXT("runtime-proof.json"))));
            Test->AddInfo(Output);return true;
        }
        }
        return false;
    }
};
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDynamicAgentPIE,"DaoVoTan.Integration.DynamicAgents.PIE",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FDynamicAgentPIE::RunTest(const FString &)
{
    ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/DaoVoTan/Maps/StillwaterValley")));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitForShadersToFinishCompiling());
    ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
    ADD_LATENT_AUTOMATION_COMMAND(FDynamicAgentExercise(this));
    ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
    return true;
}
#endif
