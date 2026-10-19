#include "Gate1AuthorCommandlet.h"
#if WITH_EDITOR
#include "Animation/AnimData/IAnimationDataController.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimTypes.h"
#include "Animation/Skeleton.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Editor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/Level.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/SkyLight.h"
#include "Engine/World.h"
#include "FileHelpers.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/WorldSettings.h"
#include "GateGameMode.h"
#include "Landscape.h"
#include "LandscapeInfo.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Misc/PackageName.h"
#include "PresentationRecipe.h"
#include "UObject/SavePackage.h"
#include "ValleyStage.h"
static void SaveAsset(UObject *Asset)
{
    Asset->GetOutermost()->FullyLoad();
    Asset->MarkPackageDirty();
    FAssetRegistryModule::AssetCreated(Asset);
    FSavePackageArgs A;
    A.TopLevelFlags = RF_Public | RF_Standalone;
    A.SaveFlags = SAVE_None;
    FString Path = FPackageName::LongPackageNameToFilename(
        Asset->GetOutermost()->GetName(), FPackageName::GetAssetPackageExtension());
    if (!UPackage::SavePackage(Asset->GetOutermost(), Asset, *Path, A))
        UE_LOG(LogTemp, Error, TEXT("GATE1 save failed %s"), *Path);
}
static UMaterial *Material(const TCHAR *Name, FLinearColor Color, bool Glow = false,
                           float Metal = 0)
{
    FString Package = FString(TEXT("/Game/DaoVoTan/Materials/")) + Name;
    auto *P = CreatePackage(*Package);
    P->FullyLoad();
    auto *M = NewObject<UMaterial>(P, Name, RF_Public | RF_Standalone);
    auto *C = NewObject<UMaterialExpressionVectorParameter>(M);
    C->ParameterName = TEXT("Tint");
    C->DefaultValue = Color;
    M->GetExpressionCollection().AddExpression(C);
    M->GetEditorOnlyData()->BaseColor.Expression = C;
    auto *Metallic = NewObject<UMaterialExpressionScalarParameter>(M);
    Metallic->ParameterName = TEXT("Metallic");
    Metallic->DefaultValue = Metal;
    M->GetExpressionCollection().AddExpression(Metallic);
    M->GetEditorOnlyData()->Metallic.Expression = Metallic;
    auto *Roughness = NewObject<UMaterialExpressionScalarParameter>(M);
    Roughness->ParameterName = TEXT("Roughness");
    Roughness->DefaultValue = .68f;
    M->GetExpressionCollection().AddExpression(Roughness);
    M->GetEditorOnlyData()->Roughness.Expression = Roughness;
    if (Glow)
    {
        auto *S = NewObject<UMaterialExpressionScalarParameter>(M);
        S->ParameterName = TEXT("Intensity");
        S->DefaultValue = 2;
        auto *Mul = NewObject<UMaterialExpressionMultiply>(M);
        Mul->A.Expression = C;
        Mul->B.Expression = S;
        M->GetExpressionCollection().AddExpression(S);
        M->GetExpressionCollection().AddExpression(Mul);
        M->GetEditorOnlyData()->EmissiveColor.Expression = Mul;
        M->SetShadingModel(MSM_Unlit);
    }
    M->SetMaterialUsage(MATUSAGE_InstancedStaticMeshes);
    M->PreEditChange(nullptr);
    M->PostEditChange();
    SaveAsset(M);
    return M;
}
static void Meditation(const TCHAR *Name, USkeletalMesh *Mesh, UAnimSequence *Idle, int32 Mode)
{
    FString Package = FString(TEXT("/Game/DaoVoTan/Animation/")) + Name;
    auto *AnimPackage = CreatePackage(*Package);
    AnimPackage->FullyLoad();
    auto *A = NewObject<UAnimSequence>(AnimPackage, Name, RF_Public | RF_Standalone);
    A->SetSkeleton(Mesh->GetSkeleton());
    auto &Ctrl = A->GetController();
    Ctrl.OpenBracket(FText::FromString(TEXT("Original meditation pose")), false);
    Ctrl.InitializeModel();
    Ctrl.SetFrameRate(FFrameRate(30, 1), false);
    const int32 Frames = Mode == 1 ? 60 : 36;
    Ctrl.SetNumberOfFrames(FFrameNumber(Frames), false);
    const FReferenceSkeleton &Ref = Mesh->GetRefSkeleton();
    for (int32 B = 0; B < Ref.GetNum(); ++B)
    {
        FName Bone = Ref.GetBoneName(B);
        FTransform Base = Ref.GetRefBonePose()[B];
        int32 SB = Mesh->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(Bone);
        if (Idle && SB != INDEX_NONE)
            Idle->GetBoneTransform(Base, FSkeletonPoseBoneIndex(SB), FAnimExtractContext(0.0f),
                                   true);
        TArray<FVector3f> Pos, Scale;
        TArray<FQuat4f> Rot;
        for (int32 F = 0; F <= Frames; ++F)
        {
            float T = float(F) / Frames;
            float Alpha = Mode == 1 ? 1.f
                                    : (Mode == 0 ? FMath::SmoothStep(0.f, 1.f, T)
                                                 : 1 - FMath::SmoothStep(0.f, 1.f, T));
            FTransform P = Base;
            FRotator Bend = FRotator::ZeroRotator;
            FString N = Bone.ToString();
            // Solved against MM_Idle in component space. Mirrored bones have different
            // local axes; applying the same roll to both legs twists rather than folds them.
            // Keep this target in sync with Tools/PlayerQ1Recovery/meditation-pose.json.
            FQuat LegTarget = Base.GetRotation();
            bool bLegTarget = true;
            if (N == TEXT("thigh_l")) LegTarget = FQuat(-.166291473, -.156067716, -.289374313, .929651828);
            else if (N == TEXT("calf_l")) LegTarget = FQuat(-.021975139, -.085987629, .905177982, .415663376);
            else if (N == TEXT("foot_l")) LegTarget = FQuat(.163307981, .115409775, -.681042680, .704408941);
            else if (N == TEXT("thigh_r")) LegTarget = FQuat(.087286798, .043001791, -.895640338, -.434004892);
            else if (N == TEXT("calf_r")) LegTarget = FQuat(.045225800, -.475945340, .786412522, .391134256);
            else if (N == TEXT("foot_r")) LegTarget = FQuat(-.043069483, .470957931, -.416029017, .776700395);
            else bLegTarget = false;
            if (N == TEXT("pelvis"))
                P.AddToTranslation(FVector(0, 0, -48 * Alpha + 36 * Alpha * (1 - Alpha)));
            if (N == TEXT("upperarm_l"))
                Bend = FRotator(0, -12 * Alpha, 15 * Alpha);
            if (N == TEXT("upperarm_r"))
                Bend = FRotator(0, 12 * Alpha, -15 * Alpha);
            if (N == TEXT("lowerarm_l"))
                Bend = FRotator(0, 0, -60 * Alpha);
            if (N == TEXT("lowerarm_r"))
                Bend = FRotator(0, 0, 60 * Alpha);
            if (N == TEXT("spine_02") && Mode == 1)
                Bend.Pitch = FMath::Sin(T * 2 * PI) * 1.5f;
            P.SetRotation((Base.GetRotation() * Bend.Quaternion()).GetNormalized());
            if (bLegTarget)
                P.SetRotation(FQuat::Slerp(Base.GetRotation(), LegTarget, Alpha).GetNormalized());
            Pos.Add(FVector3f(P.GetTranslation()));
            Rot.Add(FQuat4f(P.GetRotation()));
            Scale.Add(FVector3f(P.GetScale3D()));
        }
        Ctrl.AddBoneCurve(Bone, false);
        Ctrl.SetBoneTrackKeys(Bone, Pos, Rot, Scale, false);
    }
    Ctrl.NotifyPopulated();
    Ctrl.CloseBracket(false);
    A->PostEditChange();
    SaveAsset(A);
}
#endif
UGate1AuthorCommandlet::UGate1AuthorCommandlet()
{
    IsClient = false;
    IsEditor = true;
    LogToConsole = true;
}
int32 UGate1AuthorCommandlet::Main(const FString &Params)
{
#if WITH_EDITOR
    Material(TEXT("M_Color"), FLinearColor(.1f, .3f, .25f));
    Material(TEXT("M_Aura"), FLinearColor(.1f, .8f, .6f), true);
    Material(TEXT("M_Ground"), FLinearColor(.13f, .21f, .16f));
    Material(TEXT("M_Rock"), FLinearColor(.18f, .25f, .26f));
    Material(TEXT("M_Stone"), FLinearColor(.35f, .38f, .33f));
    Material(TEXT("M_Wood"), FLinearColor(.16f, .075f, .035f));
    Material(TEXT("M_Leaf"), FLinearColor(.035f, .17f, .12f));
    Material(TEXT("M_Gold"), FLinearColor(.65f, .38f, .09f), false, .65f);
    Material(TEXT("M_Steel"), FLinearColor(.5f, .65f, .68f), false, .9f);
    auto *Mesh = LoadObject<USkeletalMesh>(
        nullptr, TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple"));
    auto *Idle = LoadObject<UAnimSequence>(
        nullptr, TEXT("/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle"));
    if (!Mesh || !Idle)
        return 2;
    Meditation(TEXT("AS_MeditationEnter"), Mesh, Idle, 0);
    Meditation(TEXT("AS_MeditationLoop"), Mesh, Idle, 1);
    Meditation(TEXT("AS_MeditationExit"), Mesh, Idle, 2);
    const TCHAR *Names[] = {TEXT("DA_Player"), TEXT("DA_NPC_A"), TEXT("DA_NPC_B")};
    for (int32 I = 0; I < 3; ++I)
    {
        FString P = FString(TEXT("/Game/DaoVoTan/Data/")) + Names[I];
        auto *R =
            NewObject<UPresentationRecipe>(CreatePackage(*P), Names[I], RF_Public | RF_Standalone);
        R->DisplayName = FText::FromString(I == 0   ? TEXT("Li Wei")
                                           : I == 1 ? TEXT("Shen / Jade Keeper")
                                                    : TEXT("Yun / Ember Pilgrim"));
        R->RealmLabel = FText::FromString(TEXT("Foundation Establishment / fixture"));
        R->Body = FSoftObjectPath(
            I == 2 ? TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple.SKM_Quinn_Simple")
                   : TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
        R->GarmentColor = I == 2   ? FLinearColor(.4f, .08f, .045f)
                          : I == 1 ? FLinearColor(.06f, .27f, .19f)
                                   : FLinearColor(.07f, .17f, .25f);
        R->bHeadwear = I == 1;
        R->bAccessory = I == 2;
        R->bSword = I != 1;
        SaveAsset(R);
    }
    UWorld *W = UEditorLoadingAndSavingUtils::NewBlankMap(false);
    if (!W)
        return 3;
    W->GetWorldSettings()->DefaultGameMode = AGateGameMode::StaticClass();
    auto *Land = W->SpawnActor<ALandscape>();
    Land->SetActorLocation(FVector(-18900, -12600, -100));
    Land->SetActorScale3D(FVector(150, 100, 100));
    Land->LandscapeMaterial =
        LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/DaoVoTan/Materials/M_Ground"));
    TMap<FGuid, TArray<uint16>> Heights;
    TArray<uint16> &H = Heights.Add(FGuid());
    H.SetNum(253 * 253);
    for (int32 Y = 0; Y < 253; ++Y)
        for (int32 X = 0; X < 253; ++X)
        {
            float WY = (Y - 126) * 100;
            float Side = FMath::Square(FMath::Max(0.f, FMath::Abs(WY) - 2700)) * 0.000065f;
            float Noise = FMath::Sin(X * .13f) * FMath::Sin(Y * .1f) * 30;
            H[Y * 253 + X] =
                FMath::Clamp(FMath::RoundToInt(32768 + (Side + Noise) * 1.28f), 0, 65535);
        }
    TMap<FGuid, TArray<FLandscapeImportLayerInfo>> Layers;
    Layers.Add(FGuid());
    Land->Import(FGuid::NewGuid(), 0, 0, 252, 252, 1, 63, Heights, nullptr, Layers,
                 ELandscapeImportAlphamapType::Additive, TArrayView<const FLandscapeLayer>());
    W->SpawnActor<AValleyStage>()->SetActorLocation(FVector(0, 0, -100));
    auto *Start = W->SpawnActor<APlayerStart>();
    Start->SetActorLocation(FVector(-12500, 0, 140));
    Start->SetActorRotation(FRotator(0, 0, 0));
    auto *Sun = W->SpawnActor<ADirectionalLight>();
    Sun->SetActorRotation(FRotator(-28, -35, 0));
    Cast<UDirectionalLightComponent>(Sun->GetLightComponent())
        ->SetMobility(EComponentMobility::Movable);
    Cast<UDirectionalLightComponent>(Sun->GetLightComponent())->SetIntensity(3.5f);
    Cast<UDirectionalLightComponent>(Sun->GetLightComponent())
        ->SetLightColor(FLinearColor(1, .86f, .65f));
    Cast<UDirectionalLightComponent>(Sun->GetLightComponent())->bAtmosphereSunLight = true;
    W->SpawnActor<ASkyAtmosphere>();
    auto *Sky = W->SpawnActor<ASkyLight>();
    Sky->GetLightComponent()->SetMobility(EComponentMobility::Movable);
    Sky->GetLightComponent()->SetIntensity(.8f);
    Sky->GetLightComponent()->bRealTimeCapture = true;
    auto *Fog = W->SpawnActor<AExponentialHeightFog>();
    Fog->GetComponent()->SetFogDensity(.008f);
    Fog->GetComponent()->SetFogHeightFalloff(.12f);
    Fog->GetComponent()->SetFogInscatteringColor(FLinearColor(.42f, .6f, .62f));
    auto *PP = W->SpawnActor<APostProcessVolume>();
    PP->bUnbound = true;
    PP->Settings.bOverride_AutoExposureMethod = true;
    PP->Settings.AutoExposureMethod = AEM_Manual;
    PP->Settings.bOverride_AutoExposureBias = true;
    PP->Settings.AutoExposureBias = 1;
    PP->Settings.bOverride_AutoExposureApplyPhysicalCameraExposure = true;
    PP->Settings.AutoExposureApplyPhysicalCameraExposure = false;
    if (!UEditorLoadingAndSavingUtils::SaveMap(W, TEXT("/Game/DaoVoTan/Maps/StillwaterValley")))
        return 4;
    UE_LOG(LogTemp, Display,
           TEXT("GATE1 authoring complete: valley + materials + recipes + 3 original meditation "
                "sequences."));
    return 0;
#else
    return 1;
#endif
}
