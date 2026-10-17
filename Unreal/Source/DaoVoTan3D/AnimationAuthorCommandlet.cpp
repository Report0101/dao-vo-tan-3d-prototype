#include "AnimationAuthorCommandlet.h"
#include "PresentationAnimationAuthoring.h"
#if WITH_EDITOR
#include "Animation/AnimSequence.h"
#include "AnimationUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "BoolColumn.h"
#include "Chooser.h"
#include "CultivatorLocomotion.h"
#include "Dom/JsonObject.h"
#include "Engine/SkeletalMesh.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "ObjectChooser_Asset.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "PoseSearch/PoseSearchDerivedData.h"
#include "PoseSearch/PoseSearchSchema.h"
#include "PoseSearch/PoseSearchFeatureChannel_Trajectory.h"
#include "PoseSearch/PoseSearchFeatureChannel_Position.h"
#include "PoseSearch/PoseSearchFeatureChannel_Velocity.h"
#include "RetargetEditor/IKRetargetBatchOperation.h"
#include "Retargeter/IKRetargeter.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/SavePackage.h"

namespace
{
template <class T> T *MakeAsset(const FString &Name)
{
    auto *Package = CreatePackage(*(TEXT("/Game/DaoVoTan/Locomotion/") + Name));
    Package->FullyLoad();
    return NewObject<T>(Package, *Name, RF_Public | RF_Standalone);
}
bool SaveLocomotionAsset(UObject *Asset)
{
    // Only newly authored target packages may be persisted. No source package
    // or accepted visual package can pass this guard.
    if (!Asset->GetPackage()->GetName().StartsWith(TEXT("/Game/DaoVoTan/Locomotion/")))
        return false;
    FString File = FPackageName::LongPackageNameToFilename(
        Asset->GetPackage()->GetName(), FPackageName::GetAssetPackageExtension());
    File = FPaths::ConvertRelativePathToFull(File);
    const FString Target = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() /
                                                             TEXT("DaoVoTan/Locomotion/"));
    if (!File.StartsWith(Target))
        return false;
    Asset->MarkPackageDirty();
    FAssetRegistryModule::AssetCreated(Asset);
    FSavePackageArgs Args;
    Args.TopLevelFlags = RF_Public | RF_Standalone;
    return UPackage::SavePackage(Asset->GetPackage(), Asset, *File, Args);
}
} // namespace
#endif
UAnimationAuthorCommandlet::UAnimationAuthorCommandlet()
{
    IsEditor = IsClient = true;
    IsServer = false;
    LogToConsole = true;
}
int32 UAnimationAuthorCommandlet::Main(const FString &Params)
{
#if WITH_EDITOR
    FString Root, Selection;
    FParse::Value(*Params, TEXT("SampleRoot="), Root);
    FParse::Value(*Params, TEXT("Selection="), Selection);
    if (!FPaths::FileExists(Root / TEXT("GameAnimationSample.uproject")))
        return 2;
    FString Json;
    if (!FFileHelper::LoadFileToString(Json, *Selection))
        return 3;
    TSharedPtr<FJsonObject> Data;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Data))
        return 4;
    auto *TargetMesh = LoadObject<USkeletalMesh>(
        nullptr, TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple"));
    if (!TargetMesh)
        return 5;
    const bool bLocalOnly = FParse::Param(*Params, TEXT("LocalClipsOnly"));
    TMap<FString, FString> States;
    TArray<FAssetData> Retargeted;
    for (const auto &Entry : Data->GetArrayField(TEXT("assets")))
    {
        const FString Name =
            FPackageName::ObjectPathToObjectName(Entry->AsObject()->GetStringField(TEXT("source")));
        FString State = Entry->AsObject()->GetStringField(TEXT("state"));
        if (State == TEXT("Ground"))
            State = Name.Contains(TEXT("_Run_")) ? TEXT("Run") : TEXT("Walk");
        States.Add(Name, State);
        if (bLocalOnly)
        {
            auto *Clip = LoadObject<UAnimSequence>(
                nullptr, *(FString(TEXT("/Game/DaoVoTan/Locomotion/Clips/")) + Name));
            if (!Clip)
                return 6;
            Retargeted.Add(FAssetData(Clip));
        }
    }
    if (!bLocalOnly)
    {
        const FString SourceContent = Root / TEXT("Content/");
        FPackageName::RegisterMountPoint(TEXT("/Game/"), SourceContent);
        auto *SourceMesh = LoadObject<USkeletalMesh>(
            nullptr, TEXT("/Game/Characters/UEFN_Mannequin/Meshes/SKM_UEFN_Mannequin"));
        auto *Retargeter = LoadObject<UIKRetargeter>(
            nullptr, TEXT("/Game/Characters/UE5_Mannequins/Rigs/RTG_UEFN_to_UE5_Mannequin"));
        if (!SourceMesh || !Retargeter)
            return 7;
        FIKRetargetBatchOperationInputs Inputs;
        Inputs.SourceMesh = SourceMesh;
        Inputs.TargetMesh = TargetMesh;
        Inputs.IKRetargetAsset = Retargeter;
        Inputs.TargetPath = TEXT("/Game/DaoVoTan/Locomotion/Clips");
        Inputs.bIncludeReferencedAssets = false;
        Inputs.bOverwriteExistingFiles = true;
        for (const auto &Entry : Data->GetArrayField(TEXT("assets")))
        {
            const FString Path = Entry->AsObject()->GetStringField(TEXT("source"));
            auto *Clip = LoadObject<UAnimSequence>(nullptr, *Path);
            if (!Clip)
                return 6;
            Inputs.AssetsToRetarget.Add(FAssetData(Clip));
        }
        // Resolve all source references before restoring the destination mount.
        if (!SourceMesh || !Retargeter)
            return 7;
        FPackageName::UnRegisterMountPoint(TEXT("/Game/"), SourceContent);
        FString Probe = FPaths::ConvertRelativePathToFull(FPackageName::LongPackageNameToFilename(
            TEXT("/Game/DaoVoTan/Locomotion/Clips/Probe"), TEXT(".uasset")));
        if (!Probe.StartsWith(FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir())))
            return 8;
        Retargeted = UIKRetargetBatchOperation::RunBatchRetarget(Inputs);
        if (Retargeted.Num() != Inputs.AssetsToRetarget.Num())
            return 9;
    }
    auto *Schema = MakeAsset<UPoseSearchSchema>(TEXT("PSS_Cultivator"));
    Schema->AddSkeleton(TargetMesh->GetSkeleton());
    Schema->SampleRate = 30;
    auto *Trajectory = NewObject<UPoseSearchFeatureChannel_Trajectory>(Schema);
    for (auto &Sample : Trajectory->Samples)
    {
        if (EnumHasAnyFlags(Sample.Flags, EPoseSearchTrajectoryFlags::PositionXY))
            Sample.Flags = (Sample.Flags & ~int32(EPoseSearchTrajectoryFlags::PositionXY)) |
                           int32(EPoseSearchTrajectoryFlags::Position);
        if (EnumHasAnyFlags(Sample.Flags, EPoseSearchTrajectoryFlags::VelocityXY))
            Sample.Flags = (Sample.Flags & ~int32(EPoseSearchTrajectoryFlags::VelocityXY)) |
                           int32(EPoseSearchTrajectoryFlags::Velocity);
    }
    Schema->AddChannel(Trajectory);
    for (FName Bone : {FName(TEXT("foot_l")), FName(TEXT("foot_r")), FName(TEXT("pelvis"))})
    {
        auto *Position = NewObject<UPoseSearchFeatureChannel_Position>(Schema);
        Position->Bone.BoneName = Bone;
        Schema->AddChannel(Position);
        auto *Velocity = NewObject<UPoseSearchFeatureChannel_Velocity>(Schema);
        Velocity->Bone.BoneName = Bone;
        Schema->AddChannel(Velocity);
    }
    Schema->PostEditChange();
    if (!SaveLocomotionAsset(Schema))
        return 10;
    TMap<FString, UPoseSearchDatabase *> Databases;
    for (const TCHAR *State :
         {TEXT("Air"), TEXT("Landing"), TEXT("Turn"), TEXT("Run"), TEXT("Walk"), TEXT("Idle")})
    {
        auto *DB = MakeAsset<UPoseSearchDatabase>(FString(TEXT("PSD_")) + State);
        DB->Schema = Schema;
        Databases.Add(State, DB);
    }
    for (const auto &Entry : Retargeted)
    {
        auto *Clip = CastChecked<UAnimSequence>(Entry.GetAsset());
        // The sample's gameplay/foley notify Blueprint classes are intentionally
        // not migrated. Root motion is queried by Pose Search, then discarded
        // by our AnimInstance rather than moving a gameplay actor.
        Clip->Notifies.Empty();
        Clip->bEnableRootMotion = true;
        Clip->bForceRootLock = false;
        Clip->BoneCompressionSettings =
            FAnimationUtils::GetDefaultAnimationBoneCompressionSettings();
        Clip->CurveCompressionSettings =
            FAnimationUtils::GetDefaultAnimationCurveCompressionSettings();
        Clip->SetPreviewMesh(nullptr);
        Clip->PostEditChange();
        if (!SaveLocomotionAsset(Clip))
            return 11;
        FPoseSearchDatabaseAnimationAsset Asset;
        Asset.AnimAsset = Clip;
        if (States.FindChecked(Clip->GetName()) == TEXT("Landing"))
            Asset.SetSamplingRange(PresentationAnimationAuthoring::LandingImpactRange(Clip));
        Databases[States.FindChecked(Clip->GetName())]->AddAnimationAsset(Asset);
    }
    for (auto &Pair : Databases)
    {
        Pair.Value->PostEditChange();
        using namespace UE::PoseSearch;
        if (FAsyncPoseSearchDatabasesManagement::RequestAsyncBuildIndex(
                Pair.Value,
                ERequestAsyncBuildFlag::NewRequest | ERequestAsyncBuildFlag::WaitForCompletion) !=
            EAsyncBuildIndexResult::Success)
            return 14;
        if (!SaveLocomotionAsset(Pair.Value))
            return 12;
    }
    auto *Table = MakeAsset<UChooserTable>(TEXT("CHT_Locomotion"));
    Table->OutputObjectType = UPoseSearchDatabase::StaticClass();
    FInstancedStruct Context;
    Context.InitializeAs<FContextObjectTypeClass>();
    Context.GetMutable<FContextObjectTypeClass>().Class = UCultivatorLocomotion::StaticClass();
    Table->ContextData.Add(Context);
    for (const TCHAR *State :
         {TEXT("Air"), TEXT("Landing"), TEXT("Turn"), TEXT("Run"), TEXT("Walk"), TEXT("Idle")})
    {
        FInstancedStruct Result;
        Result.InitializeAs<FAssetChooser>();
        Result.GetMutable<FAssetChooser>().Asset = Databases[State];
        Table->ResultsStructs.Add(Result);
    }
    auto Column = [&](FName Name, TArray<EBoolColumnCellValue> Values)
    {
        FInstancedStruct Value;
        Value.InitializeAs<FBoolColumn>();
        auto &C = Value.GetMutable<FBoolColumn>();
        C.InputValue.InitializeAs<FBoolContextProperty>();
        auto &Binding = C.InputValue.GetMutable<FBoolContextProperty>().Binding;
        Binding.PropertyBindingChain.Add(Name);
        Binding.ContextIndex = 0;
        C.RowValuesWithAny = Values;
        Table->ColumnsStructs.Add(Value);
    };
    using E = EBoolColumnCellValue;
    Column(TEXT("bAirborne"), {E::MatchTrue, E::MatchFalse, E::MatchFalse, E::MatchFalse,
                               E::MatchFalse, E::MatchFalse});
    Column(TEXT("bLanding"),
           {E::MatchAny, E::MatchTrue, E::MatchFalse, E::MatchFalse, E::MatchFalse, E::MatchFalse});
    Column(TEXT("bTurning"),
           {E::MatchAny, E::MatchAny, E::MatchTrue, E::MatchAny, E::MatchAny, E::MatchAny});
    Column(TEXT("bMoving"),
           {E::MatchAny, E::MatchAny, E::MatchFalse, E::MatchTrue, E::MatchTrue, E::MatchFalse});
    Column(TEXT("bRunning"),
           {E::MatchAny, E::MatchAny, E::MatchAny, E::MatchTrue, E::MatchFalse, E::MatchAny});
    Table->Compile(true);
    if (!SaveLocomotionAsset(Table))
        return 13;
    UE_LOG(LogTemp, Display,
           TEXT("ANIMATION minimum migration: %d retained clips, 6 databases, 1 schema, 1 Chooser. "
                "Source packages never saved."),
           Retargeted.Num());
    return 0;
#else
    return 1;
#endif
}
