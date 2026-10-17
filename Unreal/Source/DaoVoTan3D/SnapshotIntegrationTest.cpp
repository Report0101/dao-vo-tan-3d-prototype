#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS
#include "AuraComponent.h"
#include "Dom/JsonObject.h"
#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameModeBase.h"
#include "GateCharacter.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "PresentationActions.h"
#include "PresentationRecipe.h"
#include "PresentationSnapshot.h"
#include "Serialization/JsonSerializer.h"
#include "SnapshotPresentationAdapter.h"
#include "Tests/AutomationEditorCommon.h"
#include "UnrealClient.h"
#include "PresentationEvidence.h"
namespace
{
FString Fixture(const TCHAR *Name)
{
    FString Text;
    FFileHelper::LoadFileToString(Text,
                                  *FPaths::Combine(FPaths::ProjectDir(),
                                                   TEXT("../Bridge/sample-data/integration-gate1"),
                                                   FString(Name) + TEXT(".json")));
    return Text;
}
FString Change(const FString &Input, const TCHAR *Key, const TSharedPtr<FJsonValue> &Value)
{
    TSharedPtr<FJsonObject> O;
    FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Input), O);
    O->SetField(Key, Value);
    FString Out;
    FJsonSerializer::Serialize(O.ToSharedRef(), TJsonWriterFactory<>::Create(&Out));
    return Out;
}
} // namespace
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSnapshotProtocolTest, "DaoVoTan.Integration.SnapshotProtocol",
                                 EAutomationTestFlags::EditorContext |
                                     EAutomationTestFlags::EngineFilter)
bool FSnapshotProtocolTest::RunTest(const FString &Params)
{
    FString Error;
    FWorldPresentationSnapshot Out;
    const FString Valid = Fixture(TEXT("initial"));
    TestTrue(TEXT("Real simulation JSON accepted"), SnapshotV1::Parse(Valid, Out, Error));
    TestEqual(TEXT("Three real actor IDs"), Out.Actors.Num(), 3);
    const FString WorldBefore = Out.WorldId;
    const TArray<TPair<FString, TSharedPtr<FJsonValue>>> Bad = {
        {TEXT("schemaVersion"), MakeShared<FJsonValueNumber>(2)},
        {TEXT("sequence"), MakeShared<FJsonValueNumber>(-1)},
        {TEXT("sequence"), MakeShared<FJsonValueNumber>(1.5)},
        {TEXT("worldTick"), MakeShared<FJsonValueNumber>(9007199254740992.0)},
        {TEXT("streamId"), MakeShared<FJsonValueString>(TEXT(""))},
        {TEXT("controlledActorId"), MakeShared<FJsonValueString>(TEXT("bad"))},
        {TEXT("actors"), MakeShared<FJsonValueString>(TEXT("not an array"))}};
    for (const auto &B : Bad)
    {
        TestFalse(TEXT("Malformed field rejected"),
                  SnapshotV1::Parse(Change(Valid, *B.Key, B.Value), Out, Error));
        TestEqual(TEXT("No partial parsed output"), Out.WorldId, WorldBefore);
    }
    FString BadEnum = Valid.Replace(TEXT("\"alive\""), TEXT("\"zombie\""));
    TestFalse(TEXT("Bad life enum rejected"), SnapshotV1::Parse(BadEnum, Out, Error));
    TestFalse(TEXT("Extra ID separators rejected"),
              SnapshotV1::Parse(Valid.Replace(TEXT("bridge1:agent:"), TEXT("bridge1::agent:")), Out,
                                Error));
    TestFalse(TEXT("Malformed JSON rejected"), SnapshotV1::Parse(TEXT("{"), Out, Error));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSnapshotPIETest, "DaoVoTan.Integration.ReadOnlyPIE",
                                 EAutomationTestFlags::EditorContext |
                                     EAutomationTestFlags::EngineFilter)
class FSnapshotExercise : public IAutomationLatentCommand
{
    FAutomationTestBase *Test;
    int32 Stage = 0;
    double Since = FPlatformTime::Seconds(), Started = Since;
    TMap<FString, AGateCharacter *> Original;
    TMap<FString, FString> Seeds;
    TMap<FString, FLinearColor> Colors;
    TMap<FString, FString> VisualSignatures;
    FString Signature(const UPresentationRecipe *R) const
    {
        return FString::Printf(TEXT("%d/%d/%d/%d/%s/%s"), R->HeadVariant, R->HairVariant,
                               R->GarmentVariant, R->AccessoryVariant, *R->SkinColor.ToString(),
                               *R->HairColor.ToString());
    }
    double ParseMs = 0, ApplyMs = 0;
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
        return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("IntegrationEvidence"));
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
    explicit FSnapshotExercise(FAutomationTestBase *T) : Test(T)
    {
    }
    virtual bool Update() override
    {
        const double Now = FPlatformTime::Seconds();
        if (Now - Started > 180)
        {
            Test->AddError(TEXT("Snapshot PIE timed out"));
            return true;
        }
        auto *W = World();
        if (!W || !W->GetAuthGameMode())
            return false;
        auto *A = W->GetAuthGameMode()->FindComponentByClass<USnapshotPresentationAdapter>();
        if (!A)
            return false;
        FString Error;
        const double T = Now - Since;
        switch (Stage)
        {
        case 0:
            if (T < 3)
                return false;
            Test->TestTrue(
                TEXT("Initial file full resync accepted"),
                A->LoadSnapshotFile(
                    FPaths::Combine(FPaths::ProjectDir(),
                                    TEXT("../Bridge/sample-data/integration-gate1/initial.json")),
                    true));
            Test->TestEqual(TEXT("Three bindings"), A->BindingCount(), 3);
            if (A->BindingCount() != 3)
                return true;
            for (const auto &S : A->Snapshot().Actors)
            {
                Original.Add(S.ActorId, A->ActorFor(S.ActorId));
                Seeds.Add(S.ActorId, S.AppearanceSeed);
                Colors.Add(S.ActorId, A->ActorFor(S.ActorId)->Recipe->GarmentColor);
                VisualSignatures.Add(S.ActorId, Signature(A->ActorFor(S.ActorId)->Recipe));
                Test->TestEqual(TEXT("Truthful name fallback"), S.DisplayName, S.ActorId);
            }
            Test->TestFalse(TEXT("Same snapshot/old sequence rejected"),
                            A->ApplyJson(Fixture(TEXT("initial")), false, Error));
            Test->TestEqual(TEXT("No duplicated bindings"), A->BindingCount(), 3);
            Shot(TEXT("01-real-ids-ui.png"));
            Next();
            break;
        case 1:
            if (T < 1)
                return false;
            Test->TestTrue(TEXT("Committed completion accepted"),
                           A->ApplyJson(Fixture(TEXT("completion")), false, Error));
            Test->TestEqual(TEXT("Two unique committed EventIds"), A->UniqueCueCount, 2);
            Next();
            break;
        case 2:
            if (T < 1.8)
                return false;
            Test->TestEqual(TEXT("Exactly one preview per unique event"), A->PlayedPreviewCount, 2);
            {
                int32 Active = 0;
                for (const auto &S : A->Snapshot().Actors)
                {
                    auto *Actor = A->ActorFor(S.ActorId);
                    if (Actor && Actor->Aura->IsEmitting())
                        ++Active;
                }
                Test->TestEqual(TEXT("Both completion actors have visible aura preview"), Active,
                                2);
            }
            Shot(TEXT("02-committed-cultivation-cues.png"));
            Next();
            break;
        case 3:
            if (T < 1)
                return false;
            Test->TestTrue(TEXT("New sequence resending same EventIds accepted"),
                           A->ApplyJson(Fixture(TEXT("resend")), false, Error));
            Test->TestEqual(TEXT("Duplicate events ignored"), A->DuplicateCueCount, 2);
            Test->TestEqual(TEXT("No extra playback"), A->PlayedPreviewCount, 2);
            {
                const int64 Before = A->LastSequence();
                const FString Bad =
                    Fixture(TEXT("acknowledged")).Replace(TEXT("\"alive\""), TEXT("\"zombie\""));
                Test->TestFalse(TEXT("Invalid later actor rejected atomically"),
                                A->ApplyJson(Bad, false, Error));
                Test->TestEqual(TEXT("Invalid input cannot consume sequence"), A->LastSequence(),
                                Before);
            }
            Test->TestFalse(TEXT("Wrong stream rejected"),
                            A->ApplyJson(Change(Fixture(TEXT("acknowledged")), TEXT("streamId"),
                                                MakeShared<FJsonValueString>(TEXT("alien-stream"))),
                                         false, Error));
            Test->TestTrue(TEXT("Acknowledged export accepted with no cues"),
                           A->ApplyJson(Fixture(TEXT("acknowledged")), false, Error));
            Next();
            break;
        case 4:
            if (T < 2)
                return false;
            Test->TestEqual(TEXT("Duplicate cue did not replay after idle"), A->PlayedPreviewCount,
                            2);
            Test->TestTrue(TEXT("Save/load fresh-stream full resync accepted"),
                           A->ApplyJson(Fixture(TEXT("resync")), true, Error));
            for (const auto &S : A->Snapshot().Actors)
            {
                Test->TestTrue(TEXT("Retains actor binding identity"),
                               A->ActorFor(S.ActorId) == Original[S.ActorId]);
                Test->TestEqual(TEXT("Seed stable after resync"), S.AppearanceSeed,
                                Seeds[S.ActorId]);
                Test->TestTrue(TEXT("Local cosmetic recipe stable after resync"),
                               A->ActorFor(S.ActorId)->Recipe->GarmentColor == Colors[S.ActorId]);
                Test->TestEqual(TEXT("Head hair robe accessory palettes stable after resync"),
                                Signature(A->ActorFor(S.ActorId)->Recipe),
                                VisualSignatures[S.ActorId]);
            }
            Test->TestEqual(TEXT("Resync does not replay history"), A->PlayedPreviewCount, 2);
            Shot(TEXT("03-save-load-resync.png"));
            Next();
            break;
        case 5:
            if (T < 1)
                return false;
            {
                FString Missing = Change(Fixture(TEXT("resync")), TEXT("sequence"),
                                         MakeShared<FJsonValueNumber>(2));
                TSharedPtr<FJsonObject> O;
                FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Missing), O);
                auto Actors = O->GetArrayField(TEXT("actors"));
                const FString Removed = Actors.Last()->AsObject()->GetStringField(TEXT("actorId"));
                Actors.Pop();
                O->SetArrayField(TEXT("actors"), Actors);
                FString Json;
                FJsonSerializer::Serialize(O.ToSharedRef(), TJsonWriterFactory<>::Create(&Json));
                Test->TestTrue(TEXT("Projection removal accepted"),
                               A->ApplyJson(Json, false, Error));
                Test->TestEqual(TEXT("Missing actor unbound"), A->BindingCount(), 2);
                Test->TestNull(TEXT("No binding for absent actor"), A->ActorFor(Removed));
                Test->TestTrue(TEXT("Unbound slot hidden without death inference"),
                               Original[Removed]->IsHidden());
                TSharedPtr<FJsonObject> Full;
                FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Fixture(TEXT("resync"))),
                                             Full);
                O->SetArrayField(TEXT("actors"), Full->GetArrayField(TEXT("actors")));
                for (int32 I = 0; I < 100; ++I)
                {
                    O->SetNumberField(TEXT("sequence"), 3 + I);
                    FString Sample;
                    FJsonSerializer::Serialize(O.ToSharedRef(),
                                               TJsonWriterFactory<>::Create(&Sample));
                    if (A->ApplyJson(Sample, false, Error))
                    {
                        ParseMs += A->LastParseMs;
                        ApplyMs += A->LastApplyMs;
                        ++Samples;
                    }
                }
            }
            {
                int32 Count = 0;
                for (TActorIterator<AGateCharacter> I(W); I; ++I)
                    ++Count;
                Test->TestEqual(TEXT("No actor spawning from repeated updates"), Count, 3);
            }
            {
                TSharedPtr<FJsonObject> Life;
                FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Fixture(TEXT("resync"))),
                                             Life);
                Life->SetNumberField(TEXT("sequence"), 103);
                const auto &Actors = Life->GetArrayField(TEXT("actors"));
                Actors[1]->AsObject()->SetStringField(TEXT("lifeStatus"), TEXT("unknown"));
                Actors[2]->AsObject()->SetStringField(TEXT("lifeStatus"), TEXT("dead"));
                FString Json;
                FJsonSerializer::Serialize(Life.ToSharedRef(), TJsonWriterFactory<>::Create(&Json));
                Test->TestTrue(TEXT("Distinct unknown/dead states accepted"),
                               A->ApplyJson(Json, false, Error));
                Test->TestEqual(TEXT("Unknown preserved"), A->Snapshot().Actors[1].LifeStatus,
                                FString(TEXT("unknown")));
                Test->TestEqual(TEXT("Dead local movement disabled"),
                                A->ActorFor(A->Snapshot().Actors[2].ActorId)
                                    ->GetCharacterMovement()
                                    ->MovementMode,
                                EMovementMode::MOVE_None);
                Test->TestEqual(TEXT("Life states do not remove bindings"), A->BindingCount(), 3);
            }
            Next();
            break;
        default:
        {
            FString Summary = FString::Printf(
                TEXT("parse_validation_mean_ms=%.6f\nbinding_update_mean_ms=%.6f\nsamples=%"
                     "d\nunique_cues=%d\nplayed_previews=%d\nduplicate_cues_ignored=%d\n"),
                ParseMs / FMath::Max(1, Samples), ApplyMs / FMath::Max(1, Samples), Samples,
                A->UniqueCueCount, A->PlayedPreviewCount, A->DuplicateCueCount);
            FFileHelper::SaveStringToFile(Summary,
                                          *FPaths::Combine(Dir(), TEXT("adapter-proof.txt")));
            Test->AddInfo(Summary);
            return true;
        }
        }
        return false;
    }
};
bool FSnapshotPIETest::RunTest(const FString &Params)
{
    ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/DaoVoTan/Maps/StillwaterValley")));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitForShadersToFinishCompiling());
    ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
    ADD_LATENT_AUTOMATION_COMMAND(FSnapshotExercise(this));
    ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
    return true;
}
#endif
