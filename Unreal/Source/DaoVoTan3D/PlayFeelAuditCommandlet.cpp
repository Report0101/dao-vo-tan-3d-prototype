#include "PlayFeelAuditCommandlet.h"
#include "GateCharacter.h"
#include "PresentationAnimationAuthoring.h"
#if WITH_EDITOR
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Animation/AnimData/IAnimationDataController.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/PackageName.h"
#include "UObject/SavePackage.h"
#include "Math/RotationMatrix.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "PoseSearch/PoseSearchDerivedData.h"

namespace PresentationAnimationAuthoring
{
FFloatInterval LandingImpactRange(UAnimSequence *Clip)
{
    const auto &Ref = Clip->GetSkeleton()->GetReferenceSkeleton();
    const int32 Root = Ref.FindBoneIndex(TEXT("root")), Pelvis = Ref.FindBoneIndex(TEXT("pelvis"));
    check(Ref.GetParentIndex(Pelvis) == Root);
    float Minimum = MAX_flt, Impact = 0;
    for (int32 F = 0; F <= FMath::FloorToInt(Clip->GetPlayLength() * 30); ++F)
    {
        FTransform R, P;
        Clip->GetBoneTransform(R, FSkeletonPoseBoneIndex(Root), FAnimExtractContext(F / 30.), true);
        Clip->GetBoneTransform(P, FSkeletonPoseBoneIndex(Pelvis), FAnimExtractContext(F / 30.),
                               true);
        const float Height = (P * R).GetTranslation().Z;
        if (Height < Minimum)
        {
            Minimum = Height;
            Impact = F / 30.f;
        }
    }

    const FFloatInterval Range(FMath::Max(0.f, Impact - .12f),
                               FMath::Min(Clip->GetPlayLength(), Impact + .10f));
    UE_LOG(LogTemp, Display,
           TEXT("CORRECTION landing %s compression=%.4f height=%.3f range=[%.4f,%.4f]"),
           *Clip->GetName(), Impact, Minimum, Range.Min, Range.Max);
    return Range;
}
bool BoundLanding()
{
    auto *DB =
        LoadObject<UPoseSearchDatabase>(nullptr, TEXT("/Game/DaoVoTan/Locomotion/PSD_Landing"));
    if (!DB || DB->GetNumAnimationAssets() != 4)
        return false;
    TArray<FPoseSearchDatabaseAnimationAsset> Assets;
    for (int32 I = 0; I < DB->GetNumAnimationAssets(); ++I)
        Assets.Add(*DB->GetDatabaseAnimationAsset(I));
    for (auto &Entry : Assets)
    {
        auto *Clip = Cast<UAnimSequence>(Entry.GetAnimationAsset());
        if (!Clip)
            return false;
        Entry.SetSamplingRange(LandingImpactRange(Clip));
    }
    while (DB->GetNumAnimationAssets())
        DB->RemoveAnimationAssetAt(DB->GetNumAnimationAssets() - 1);
    for (const auto &Entry : Assets)
        DB->AddAnimationAsset(Entry);
    DB->PostEditChange();
    using namespace UE::PoseSearch;
    if (FAsyncPoseSearchDatabasesManagement::RequestAsyncBuildIndex(
            DB, ERequestAsyncBuildFlag::NewRequest | ERequestAsyncBuildFlag::WaitForCompletion) !=
        EAsyncBuildIndexResult::Success)
        return false;
    DB->MarkPackageDirty();
    FSavePackageArgs Args;
    Args.TopLevelFlags = RF_Public | RF_Standalone;
    const FString File = FPackageName::LongPackageNameToFilename(
        DB->GetPackage()->GetName(), FPackageName::GetAssetPackageExtension());
    return UPackage::SavePackage(DB->GetPackage(), DB, *File, Args);
}
struct FKey
{
    float T;
    FVector Hand;
    FVector Blade;
    float Hip;
    float Torso;
};
// Component-space key poses describe an outside-body cutting arc, not weapon offsets.
const FKey Keys[] = {{0, FVector(-34, 32, 115), FVector(-.12, .90, .42), 0, 0},
                     {.14f, FVector(-38, 27, 132), FVector(-.35, .65, .68), -6, -4},
                     {.30f, FVector(-40, 34, 132), FVector(-.60, .80, .10), -6, -12},
                     {.52f, FVector(-6, 43, 113), FVector(.70, .68, -.20), 10, 18},
                     {.68f, FVector(8, 33, 116), FVector(.70, .70, -.14), 12, 22},
                     {.82f, FVector(-20, 35, 115), FVector(.25, .90, .35), 5, 8},
                     {1, FVector(-34, 32, 115), FVector(-.12, .90, .42), 0, 0}};
bool BakeJian()
{
    auto *Mesh = LoadObject<USkeletalMesh>(
        nullptr, TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple"));
    auto *Idle = LoadObject<UAnimSequence>(
        nullptr, TEXT("/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle"));
    if (!Mesh || !Idle)
        return false;
    const auto &Ref = Mesh->GetRefSkeleton();
    TArray<FTransform> Base;
    for (int32 B = 0; B < Ref.GetNum(); ++B)
    {
        FTransform P = Ref.GetRefBonePose()[B];
        const int32 S =
            Idle->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(Ref.GetBoneName(B));
        if (S != INDEX_NONE)
            Idle->GetBoneTransform(P, FSkeletonPoseBoneIndex(S), FAnimExtractContext(0.), true);
        Base.Add(P);
    }
    for (bool Attack : {false, true})
    {
        const FString Path = Attack ? TEXT("/Game/DaoVoTan/Visual11/AS_SwordAttackA")
                                    : TEXT("/Game/DaoVoTan/Visual11/AS_SwordIdle");
        auto *Clip = LoadObject<UAnimSequence>(nullptr, *Path);
        if (!Clip || Clip->GetSkeleton() != Mesh->GetSkeleton())
            return false;
        const int32 Frames = Attack ? 42 : 30;
        TArray<TArray<FTransform>> Poses;
        for (int32 F = 0; F <= Frames; ++F)
        {
            const float T = Attack ? float(F) / Frames : 0;
            int32 K = 0;
            while (K < UE_ARRAY_COUNT(Keys) - 2 && T > Keys[K + 1].T)
                ++K;
            const float U =
                FMath::SmoothStep(0.f, 1.f, (T - Keys[K].T) / (Keys[K + 1].T - Keys[K].T));
            const FVector Hand = FMath::Lerp(Keys[K].Hand, Keys[K + 1].Hand, U);
            const FVector Direction =
                FMath::Lerp(Keys[K].Blade, Keys[K + 1].Blade, U).GetSafeNormal();
            TArray<FTransform> Local = Base, Global;
            auto FK = [&]()
            {
                Global.Reset();
                for (int32 I = 0; I < Local.Num(); ++I)
                {
                    const int32 P = Ref.GetParentIndex(I);
                    Global.Add(P == INDEX_NONE ? Local[I] : Local[I] * Global[P]);
                }
            };
            const int32 Hip = Ref.FindBoneIndex(TEXT("pelvis")),
                        Spine = Ref.FindBoneIndex(TEXT("spine_02"));
            Local[Hip].SetRotation(
                (Local[Hip].GetRotation() *
                 FQuat(FVector::XAxisVector,
                       FMath::DegreesToRadians(FMath::Lerp(Keys[K].Hip, Keys[K + 1].Hip, U))))
                    .GetNormalized());
            Local[Spine].SetRotation(
                (Local[Spine].GetRotation() *
                 FQuat(FVector::XAxisVector,
                       FMath::DegreesToRadians(FMath::Lerp(Keys[K].Torso, Keys[K + 1].Torso, U))))
                    .GetNormalized());
            FK();
            const int32 Upper = Ref.FindBoneIndex(TEXT("upperarm_r")),
                        Lower = Ref.FindBoneIndex(TEXT("lowerarm_r")),
                        Wrist = Ref.FindBoneIndex(TEXT("hand_r"));
            const FVector Shoulder = Global[Upper].GetTranslation();
            const float A = (Global[Lower].GetTranslation() - Shoulder).Size(),
                        B = (Global[Wrist].GetTranslation() - Global[Lower].GetTranslation())
                                .Size();
            const FVector Axis = (Hand - Shoulder).GetSafeNormal();
            const float D =
                FMath::Clamp((Hand - Shoulder).Size(), FMath::Abs(A - B) + 1.f, A + B - .5f);
            const FVector Target = Shoulder + Axis * D;
            const float Along = (A * A - B * B + D * D) / (2 * D);
            const FVector Pole = FVector(-1, .1f, .1f);
            const FVector Bend = (Pole - Axis * FVector::DotProduct(Pole, Axis)).GetSafeNormal();
            const FVector Elbow = Shoulder + Axis * Along +
                                  Bend * FMath::Sqrt(FMath::Max(0.f, A * A - Along * Along));
            auto Aim = [&](int32 Bone, int32 Child, FVector Goal)
            {
                const FQuat Delta = FQuat::FindBetweenNormals(
                    (Global[Child].GetTranslation() - Global[Bone].GetTranslation())
                        .GetSafeNormal(),
                    (Goal - Global[Bone].GetTranslation()).GetSafeNormal());
                const FQuat Desired = Delta * Global[Bone].GetRotation();
                Local[Bone].SetRotation(
                    (Global[Ref.GetParentIndex(Bone)].GetRotation().Inverse() * Desired)
                        .GetNormalized());
                FK();
            };
            Aim(Upper, Lower, Elbow);
            Aim(Lower, Wrist, Target);
            const FQuat Sword = FRotationMatrix::MakeFromZX(Direction, FVector::UpVector).ToQuat();
            const FQuat WristRotation =
                Sword * AGateCharacter::JianGripTransform().GetRotation().Inverse();
            Local[Wrist].SetRotation(
                (Global[Lower].GetRotation().Inverse() * WristRotation).GetNormalized());
            Poses.Add(MoveTemp(Local));
        }
        auto &Controller = Clip->GetController();
        Controller.OpenBracket(
            FText::FromString(TEXT("Project-authored jian guard and outside-body slash")), false);
        Controller.InitializeModel();
        Controller.RemoveAllBoneTracks(false);
        Controller.SetFrameRate(FFrameRate(30, 1), false);
        Controller.SetNumberOfFrames(FFrameNumber(Frames), false);
        for (int32 B = 0; B < Ref.GetNum(); ++B)
        {
            TArray<FVector3f> P, S;
            TArray<FQuat4f> Q;
            for (const auto &Pose : Poses)
            {
                P.Add(FVector3f(Pose[B].GetTranslation()));
                S.Add(FVector3f(Pose[B].GetScale3D()));
                Q.Add(FQuat4f(Pose[B].GetRotation()));
            }
            Controller.AddBoneCurve(Ref.GetBoneName(B), false);
            Controller.SetBoneTrackKeys(Ref.GetBoneName(B), P, Q, S, false);
        }
        Controller.NotifyPopulated();
        Controller.CloseBracket(false);
        Clip->Notifies.Reset();
        Clip->bEnableRootMotion = false;
        Clip->PostEditChange();
        Clip->MarkPackageDirty();
        FSavePackageArgs Args;
        Args.TopLevelFlags = RF_Public | RF_Standalone;
        const FString File =
            FPackageName::LongPackageNameToFilename(Path, FPackageName::GetAssetPackageExtension());
        if (!File.StartsWith(FPaths::ProjectContentDir() / TEXT("DaoVoTan/Visual11/")))
            return false;
        if (!UPackage::SavePackage(Clip->GetPackage(), Clip, *File, Args))
            return false;
    }
    return true;
}
} // namespace PresentationAnimationAuthoring
#endif
UPlayFeelAuditCommandlet::UPlayFeelAuditCommandlet()
{
    IsClient = false;
    IsServer = false;
    IsEditor = true;
    LogToConsole = true;
}
int32 UPlayFeelAuditCommandlet::Main(const FString &Params)
{
#if WITH_EDITOR
    if (FParse::Param(*Params, TEXT("AuthorSword")))
        return PresentationAnimationAuthoring::BakeJian() ? 0 : 6;
    if (FParse::Param(*Params, TEXT("AuthorLanding")))
        return PresentationAnimationAuthoring::BoundLanding() ? 0 : 7;
    FString Output;
    if (!FParse::Value(*Params, TEXT("AuditOutput="), Output))
        return 2;
    auto *Mesh = LoadObject<USkeletalMesh>(
        nullptr, TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple"));
    auto *Jian = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/DaoVoTan/Visual11/SM_Jian"));
    if (!Mesh || !Jian)
        return 3;
    auto Vec = [](FVector V)
    {
        return MakeShared<FJsonValueArray>(TArray<TSharedPtr<FJsonValue>>{
            MakeShared<FJsonValueNumber>(V.X), MakeShared<FJsonValueNumber>(V.Y),
            MakeShared<FJsonValueNumber>(V.Z)});
    };
    auto Root = MakeShared<FJsonObject>();
    Root->SetField(TEXT("meshBoundsMin"), Vec(Jian->GetBoundingBox().Min));
    Root->SetField(TEXT("meshBoundsMax"), Vec(Jian->GetBoundingBox().Max));
    const auto &Ref = Mesh->GetRefSkeleton();
    TArray<TSharedPtr<FJsonValue>> Clips;
    for (const TCHAR *Path :
         {TEXT("/Game/DaoVoTan/Visual11/AS_SwordAttackA"),
          TEXT("/Game/DaoVoTan/Visual11/AS_SwordIdle"),
          TEXT("/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle"),
          TEXT("/Game/DaoVoTan/Locomotion/Clips/M_Neutral_Jump_LL_Land_Stand_Light_Lfoot")})
    {
        auto *Clip = LoadObject<UAnimSequence>(nullptr, Path);
        if (!Clip)
            return 4;
        auto Item = MakeShared<FJsonObject>();
        Item->SetStringField(TEXT("clip"), Path);
        Item->SetNumberField(TEXT("duration"), Clip->GetPlayLength());
        TArray<TSharedPtr<FJsonValue>> Frames;
        // Dense sampling includes the specifically requested 0/10/20/30/50/75/100 percent.
        for (int32 Frame = 0; Frame <= 120; ++Frame)
        {
            const double T = Clip->GetPlayLength() * Frame / 120.;
            TArray<FTransform> Global;
            auto Bones = MakeShared<FJsonObject>();
            for (int32 I = 0; I < Ref.GetNum(); ++I)
            {
                FTransform Local = Ref.GetRefBonePose()[I];
                const int32 S =
                    Clip->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(Ref.GetBoneName(I));
                if (S != INDEX_NONE)
                    Clip->GetBoneTransform(Local, FSkeletonPoseBoneIndex(S), FAnimExtractContext(T),
                                           true);
                const int32 Parent = Ref.GetParentIndex(I);
                Global.Add(Parent == INDEX_NONE ? Local : Local * Global[Parent]);
                const FString Name = Ref.GetBoneName(I).ToString();
                if (Name == TEXT("pelvis") || Name.StartsWith(TEXT("spine_")) ||
                    Name == TEXT("head") || Name.Contains(TEXT("arm_r")) ||
                    Name == TEXT("hand_r") || Name.StartsWith(TEXT("index_")) ||
                    Name.StartsWith(TEXT("middle_")) || Name.StartsWith(TEXT("thumb_")) ||
                    Name.StartsWith(TEXT("thigh_")) || Name.StartsWith(TEXT("calf_")))
                {
                    auto B = MakeShared<FJsonObject>();
                    B->SetField(TEXT("p"), Vec(Global[I].GetTranslation()));
                    B->SetField(TEXT("x"), Vec(Global[I].GetUnitAxis(EAxis::X)));
                    B->SetField(TEXT("y"), Vec(Global[I].GetUnitAxis(EAxis::Y)));
                    B->SetField(TEXT("z"), Vec(Global[I].GetUnitAxis(EAxis::Z)));
                    Bones->SetObjectField(Name, B);
                }
            }
            const FTransform Hand = Global[Ref.FindBoneIndex(TEXT("hand_r"))];
            const FTransform Grip = AGateCharacter::JianGripTransform();
            const FTransform Sword = Grip * Hand;
            auto F = MakeShared<FJsonObject>();
            F->SetNumberField(TEXT("fraction"), Frame / 120.);
            F->SetObjectField(TEXT("bones"), Bones);
            F->SetField(TEXT("swordRoot"), Vec(Sword.GetTranslation()));
            F->SetField(TEXT("swordTip"), Vec(Sword.TransformPosition(FVector(0, 0, 101))));
            Frames.Add(MakeShared<FJsonValueObject>(F));
        }
        Item->SetArrayField(TEXT("frames"), Frames);
        Clips.Add(MakeShared<FJsonValueObject>(Item));
    }
    Root->SetArrayField(TEXT("clips"), Clips);
    FString Json;
    FJsonSerializer::Serialize(Root, TJsonWriterFactory<>::Create(&Json));
    return FFileHelper::SaveStringToFile(Json, *Output) ? 0 : 5;
#else
    return 1;
#endif
}
