#include "Visual11AuthorCommandlet.h"
#if WITH_EDITOR
#include "Animation/AnimData/IAnimationDataController.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "BoneWeights.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "FileHelpers.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionLength.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionOneMinus.h"
#include "Materials/MaterialExpressionParticleColor.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionSphereMask.h"
#include "Materials/MaterialExpressionSubtract.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "MeshDescription.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "NiagaraEditorUtilities.h"
#include "NiagaraEmitter.h"
#include "NiagaraGraph.h"
#include "NiagaraNodeFunctionCall.h"
#include "NiagaraNodeOutput.h"
#include "NiagaraParameterMapHistory.h"
#include "NiagaraScriptSource.h"
#include "NiagaraSpriteRendererProperties.h"
#include "NiagaraSystem.h"
#include "NiagaraSystemFactoryNew.h"
#include "PresentationRecipe.h"
#include "Rendering/SkeletalMeshLODModel.h"
#include "Rendering/SkeletalMeshModel.h"
#include "SkeletalMeshAttributes.h"
#include "StaticMeshAttributes.h"
#include "UObject/SavePackage.h"
#include "ValleyStage.h"
#include "ViewModels/Stack/NiagaraStackGraphUtilities.h"

namespace Visual11Author
{
void Save(UObject *A)
{
    A->MarkPackageDirty();
    FAssetRegistryModule::AssetCreated(A);
    FSavePackageArgs Args;
    Args.TopLevelFlags = RF_Public | RF_Standalone;
    const FString File = FPackageName::LongPackageNameToFilename(
        A->GetOutermost()->GetName(), FPackageName::GetAssetPackageExtension());
    check(UPackage::SavePackage(A->GetOutermost(), A, *File, Args));
}
template <typename T> T *Asset(const FString &Name)
{
    const FString Path = TEXT("/Game/DaoVoTan/Visual11/") + Name;
    UPackage *P = CreatePackage(*Path);
    P->FullyLoad();
    return NewObject<T>(P, *FPackageName::GetShortName(Path), RF_Public | RF_Standalone);
}
UMaterial *Surface(const FString &Name, FLinearColor Color, float Roughness = .7f, float Metal = 0,
                   bool Aura = false)
{
    auto *M = Asset<UMaterial>(Name);
    M->TwoSided = true;
    auto *C = NewObject<UMaterialExpressionVectorParameter>(M);
    C->ParameterName = TEXT("Tint");
    C->DefaultValue = Color;
    M->GetExpressionCollection().AddExpression(C);
    M->GetEditorOnlyData()->BaseColor.Expression = C;
    auto Scalar = [&](const TCHAR *Key, float V)
    {
        auto *S = NewObject<UMaterialExpressionScalarParameter>(M);
        S->ParameterName = Key;
        S->DefaultValue = V;
        M->GetExpressionCollection().AddExpression(S);
        return S;
    };
    M->GetEditorOnlyData()->Roughness.Expression = Scalar(TEXT("Roughness"), Roughness);
    M->GetEditorOnlyData()->Metallic.Expression = Scalar(TEXT("Metallic"), Metal);
    if (Aura)
    {
        M->BlendMode = BLEND_Additive;
        M->SetShadingModel(MSM_Unlit);
        M->SetMaterialUsage(MATUSAGE_NiagaraSprites);
        auto *Mul = NewObject<UMaterialExpressionMultiply>(M);
        Mul->A.Expression = C;
        Mul->B.Expression = Scalar(TEXT("Intensity"), 1.2f);
        M->GetExpressionCollection().AddExpression(Mul);
        M->GetEditorOnlyData()->EmissiveColor.Expression = Mul;
        auto *UV = NewObject<UMaterialExpressionTextureCoordinate>(M);
        auto *Sub = NewObject<UMaterialExpressionSubtract>(M);
        Sub->A.Expression = UV;
        Sub->ConstB = .5f;
        auto *Len = NewObject<UMaterialExpressionLength>(M);
        Len->Input.Expression = Sub;
        auto *Size = NewObject<UMaterialExpressionMultiply>(M);
        Size->A.Expression = Len;
        Size->ConstB = 2;
        auto *Inv = NewObject<UMaterialExpressionOneMinus>(M);
        Inv->Input.Expression = Size;
        auto *Alpha = NewObject<UMaterialExpressionMultiply>(M);
        Alpha->A.Expression = Inv;
        Alpha->B.Expression = Inv;
        TArray<UMaterialExpression *> Expressions = {UV, Sub, Len, Size, Inv, Alpha};
        for (auto *E : Expressions)
            M->GetExpressionCollection().AddExpression(E);
        M->GetEditorOnlyData()->Opacity.Expression = Alpha;
    }
    else
        M->SetMaterialUsage(MATUSAGE_SkeletalMesh);
    M->PostEditChange();
    Save(M);
    return M;
}
struct FVisualGeometry
{
    TArray<FVector> Points;
    TArray<FIntVector> Faces;
    TArray<int32> Materials;
    TArray<int32> Bones;
    int32 Vertex(FVector V, int32 Bone = 0)
    {
        Bones.Add(Bone);
        return Points.Add(V);
    }
    void Tri(int32 A, int32 B, int32 C, int32 M)
    {
        Faces.Add(FIntVector(A, B, C));
        Materials.Add(M);
    }
    void Ellipsoid(FVector Center, FVector Scale, int32 Mat, int32 Bone = 0, int32 Slices = 20,
                   int32 Rings = 12)
    {
        int32 Start = Points.Num();
        for (int32 J = 0; J <= Rings; ++J)
            for (int32 I = 0; I <= Slices; ++I)
            {
                float P = PI * J / Rings, A = 2 * PI * I / Slices;
                Vertex(Center + FVector(FMath::Sin(P) * FMath::Cos(A) * Scale.X,
                                        FMath::Sin(P) * FMath::Sin(A) * Scale.Y,
                                        FMath::Cos(P) * Scale.Z),
                       Bone);
            }
        for (int32 J = 0; J < Rings; ++J)
            for (int32 I = 0; I < Slices; ++I)
            {
                int32 A = Start + J * (Slices + 1) + I, B = A + Slices + 1;
                if (J > 0)
                    Tri(A, B, A + 1, Mat);
                if (J < Rings - 1)
                    Tri(A + 1, B, B + 1, Mat);
            }
    }
    void Tube(FVector A, FVector B, float RA, float RB, int32 Mat, int32 Bone, int32 BoneB = -1,
              float Oval = 1)
    {
        FVector Axis = (B - A).GetSafeNormal(), X, Y;
        Axis.FindBestAxisVectors(X, Y);
        int32 Start = Points.Num();
        const int32 S = 24, R = 8;
        for (int32 J = 0; J <= R; ++J)
            for (int32 I = 0; I <= S; ++I)
            {
                float T = float(J) / R, Ang = 2 * PI * I / S;
                Vertex(FMath::Lerp(A, B, T) + (X * FMath::Cos(Ang) * Oval + Y * FMath::Sin(Ang)) *
                                                  FMath::Lerp(RA, RB, T),
                       BoneB >= 0 && T > .5f ? BoneB : Bone);
            }
        for (int32 J = 0; J < R; ++J)
            for (int32 I = 0; I < S; ++I)
            {
                int32 A0 = Start + J * (S + 1) + I, B0 = A0 + S + 1;
                Tri(A0, A0 + 1, B0, Mat);
                Tri(A0 + 1, B0 + 1, B0, Mat);
            }
    }
    FMeshDescription Describe(int32 Slots, bool Skin)
    {
        FMeshDescription D;
        FStaticMeshAttributes A(D);
        A.Register();
        if (Skin)
        {
            FSkeletalMeshAttributes S(D);
            S.Register();
        }
        auto Positions = A.GetVertexPositions();
        auto Normals = A.GetVertexInstanceNormals();
        auto UV = A.GetVertexInstanceUVs();
        UV.SetNumChannels(1);
        TArray<FPolygonGroupID> Groups;
        for (int32 I = 0; I < Slots; ++I)
        {
            auto G = D.CreatePolygonGroup();
            Groups.Add(G);
            A.GetPolygonGroupMaterialSlotNames()[G] = FName(*FString::Printf(TEXT("Slot%d"), I));
        }
        TArray<FVertexID> Verts;
        for (int32 I = 0; I < Points.Num(); ++I)
        {
            auto V = D.CreateVertex();
            Verts.Add(V);
            Positions[V] = FVector3f(Points[I]);
            if (Skin)
            {
                UE::AnimationCore::FBoneWeight W(Bones[I], 1.f);
                FSkeletalMeshAttributes(D).GetVertexSkinWeights().Set(V, MakeArrayView(&W, 1));
            }
        }
        for (int32 I = 0; I < Faces.Num(); ++I)
        {
            auto F = Faces[I];
            FVector N = FVector::CrossProduct(Points[F.Y] - Points[F.X], Points[F.Z] - Points[F.X])
                            .GetSafeNormal();
            if (N.IsNearlyZero())
                continue;
            TArray<FVertexInstanceID> VI;
            for (int32 K : {F.X, F.Y, F.Z})
            {
                auto Id = D.CreateVertexInstance(Verts[K]);
                VI.Add(Id);
                Normals[Id] = FVector3f(N);
                UV.Set(Id, 0, FVector2f(Points[K].X * .025f, Points[K].Z * .025f));
            }
            D.CreatePolygon(Groups[Materials[I]], VI);
        }
        return D;
    }
    void Static(const FString &Name, const TArray<UMaterialInterface *> &Mats,
                const FTransform *ToLocal = nullptr)
    {
        if (ToLocal)
            for (auto &V : Points)
                V = ToLocal->InverseTransformPosition(V);
        auto *M = Asset<UStaticMesh>(Name);
        for (auto *Mat : Mats)
            M->GetStaticMaterials().Add(FStaticMaterial(Mat));
        auto D = Describe(Mats.Num(), false);
        UStaticMesh::FBuildMeshDescriptionsParams P;
        P.bBuildSimpleCollision = Name == TEXT("SM_ScholarRock");
        P.bAllowCpuAccess = true;
        check(M->BuildFromMeshDescriptions({&D}, P));
        Save(M);
    }
    void Skinned(const FString &Name, USkeletalMesh *Rig, const TArray<UMaterialInterface *> &Mats)
    {
        auto *M = Asset<USkeletalMesh>(Name);
        M->SetSkeleton(Rig->GetSkeleton());
        M->SetRefSkeleton(Rig->GetRefSkeleton());
        M->AddLODInfo();
        for (int32 I = 0; I < Mats.Num(); ++I)
            M->GetMaterials().Add(FSkeletalMaterial(Mats[I], true, false,
                                                    FName(*FString::Printf(TEXT("Slot%d"), I))));
        auto D = Describe(Mats.Num(), true);
        FSkeletalMeshAttributes A(D);
        const auto &Ref = Rig->GetRefSkeleton();
        for (int32 I = 0; I < Ref.GetNum(); ++I)
        {
            auto B = A.CreateBone();
            A.GetBoneNames()[B] = Ref.GetBoneName(I);
            A.GetBoneParentIndices()[B] = Ref.GetParentIndex(I);
            A.GetBonePoses()[B] = Ref.GetRefBonePose()[I];
        }
        M->GetImportedModel()->LODModels.Add(new FSkeletalMeshLODModel());
        M->SetImportedBounds(FBoxSphereBounds(FBox(Points.GetData(), Points.Num())));
        M->CreateMeshDescription(0, MoveTemp(D));
        M->CommitMeshDescription(0);
        M->CalculateInvRefMatrices();
        M->Build();
        Save(M);
    }
};
void SwordAnimation(USkeletalMesh *Rig)
{
    auto *Idle = LoadObject<UAnimSequence>(
        nullptr, TEXT("/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle"));
    for (int32 Mode = 0; Mode < 2; ++Mode)
    {
        auto *A = Asset<UAnimSequence>(Mode ? TEXT("AS_SwordAttackA") : TEXT("AS_SwordIdle"));
        A->SetSkeleton(Rig->GetSkeleton());
        auto &Ctrl = A->GetController();
        Ctrl.OpenBracket(FText::FromString(TEXT("Original jian guard / diagonal cut")), false);
        Ctrl.InitializeModel();
        Ctrl.SetFrameRate(FFrameRate(30, 1), false);
        const int32 Frames = Mode ? 36 : 60;
        Ctrl.SetNumberOfFrames(FFrameNumber(Frames), false);
        const auto &Ref = Rig->GetRefSkeleton();
        for (int32 B = 0; B < Ref.GetNum(); ++B)
        {
            FName Bone = Ref.GetBoneName(B);
            FTransform Base = Ref.GetRefBonePose()[B];
            int32 SB = Rig->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(Bone);
            if (SB != INDEX_NONE)
                Idle->GetBoneTransform(Base, FSkeletonPoseBoneIndex(SB), FAnimExtractContext(0.),
                                       true);
            TArray<FVector3f> P, S;
            TArray<FQuat4f> Q;
            FString N = Bone.ToString();
            for (int32 F = 0; F <= Frames; ++F)
            {
                float T = float(F) / Frames,
                      Wind = Mode ? FMath::Sin(FMath::Clamp(T / .42f, 0.f, 1.f) * PI / 2) : 0,
                      Strike = Mode ? FMath::SmoothStep(.32f, .62f, T) : 0,
                      Recover = Mode ? FMath::SmoothStep(.72f, 1.f, T) : 0;
                FRotator R = FRotator::ZeroRotator;
                float Cut = (Wind - Strike) * 80;
                if (N == TEXT("upperarm_r"))
                    R = FRotator(0, -30 + Cut, -25 - Strike * 65 + Recover * 65);
                if (N == TEXT("lowerarm_r"))
                    R = FRotator(0, 0, 55 - Cut * .35f);
                if (N == TEXT("hand_r"))
                    R = FRotator(-12, 0, -15);
                if (N == TEXT("spine_02"))
                    R = FRotator(0, (Wind - Strike) * 22, 0);
                if (N == TEXT("upperarm_l"))
                    R = FRotator(0, 15, 12);
                P.Add(FVector3f(Base.GetTranslation()));
                S.Add(FVector3f(Base.GetScale3D()));
                Q.Add(FQuat4f((Base.GetRotation() * R.Quaternion()).GetNormalized()));
            }
            Ctrl.AddBoneCurve(Bone, false);
            Ctrl.SetBoneTrackKeys(Bone, P, Q, S, false);
        }
        Ctrl.NotifyPopulated();
        Ctrl.CloseBracket(false);
        A->PostEditChange();
        Save(A);
    }
}
void Niagara(UMaterial *Mat)
{
    auto *System = Asset<UNiagaraSystem>(TEXT("NS_CultivationAura"));
    UNiagaraSystemFactoryNew::InitializeSystem(System, true);
    auto &Parameters = System->GetExposedParameters();
    for (const auto &Pair : TArray<TPair<FName, float>>{{TEXT("User.Density"), 32},
                                                        {TEXT("User.Radius"), 85},
                                                        {TEXT("User.VerticalMotion"), 18},
                                                        {TEXT("User.Lifetime"), 4},
                                                        {TEXT("User.SpriteSize"), 5},
                                                        {TEXT("User.Intensity"), 1},
                                                        {TEXT("User.Pulse"), 1}})
        Parameters.SetParameterValue<float>(
            Pair.Value, FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), Pair.Key), true);
    Parameters.SetParameterValue<FVector3f>(
        FVector3f::ZeroVector,
        FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("User.Gravity")), true);
    Parameters.SetParameterValue<FLinearColor>(
        FLinearColor(.15f, .85f, .65f),
        FNiagaraVariable(FNiagaraTypeDefinition::GetColorDef(), TEXT("User.Color")), true);
    FNiagaraVariable MaterialParameter(FNiagaraTypeDefinition::GetUMaterialDef(),
                                       TEXT("User.AuraMaterial"));
    Parameters.AddParameter(MaterialParameter);
    Parameters.SetUObject(Mat, MaterialParameter);
    auto *E = LoadObject<UNiagaraEmitter>(
        nullptr, TEXT("/Niagara/DefaultAssets/Templates/Emitters/Fountain.Fountain"));
    check(E);
    FNiagaraEditorUtilities::AddEmitterToSystem(*System, *E, E->GetExposedVersion().VersionGuid);
    for (auto &H : System->GetEmitterHandles())
        if (auto *Data = H.GetEmitterData())
        {
            Data->bLocalSpace = true;
            for (auto *R : Data->GetRenderers())
                if (auto *Sprite = Cast<UNiagaraSpriteRendererProperties>(R))
                {
                    Sprite->Material = Mat;
                    Sprite->MaterialUserParamBinding.Parameter = MaterialParameter;
                }
            auto *Source = Cast<UNiagaraScriptSource>(Data->GraphSource);
            if (!Source)
                continue;
            TArray<UNiagaraNodeFunctionCall *> Nodes;
            Source->NodeGraph->GetNodesOfClass(Nodes);
            for (auto *N : Nodes)
            {
                TArray<FNiagaraVariable> Inputs;
                TSet<FNiagaraVariable> Hidden;
                FNiagaraStackGraphUtilities::GetStackFunctionInputs(
                    *N, Inputs, Hidden,
                    FCompileConstantResolver(H.GetInstance(),
                                             ENiagaraScriptUsage::ParticleSpawnScript),
                    FNiagaraStackGraphUtilities::ENiagaraGetStackFunctionInputPinsOptions::
                        ModuleInputsOnly);
                for (const auto &Input : Inputs)
                {
                    FName User = NAME_None;
                    const FString Module = N->GetFunctionName(), Name = Input.GetName().ToString();
                    if (Module == TEXT("SpawnRate") && Name == TEXT("Module.SpawnRate"))
                        User = TEXT("User.Density");
                    if (Module == TEXT("ShapeLocation") && Name == TEXT("Module.Sphere Radius"))
                        User = TEXT("User.Radius");
                    if (Module == TEXT("AddVelocity") && Name == TEXT("Module.Velocity Speed"))
                        User = TEXT("User.VerticalMotion");
                    if (Module == TEXT("GravityForce") && Name == TEXT("Module.Gravity"))
                        User = TEXT("User.Gravity");
                    if (Module == TEXT("InitializeParticle"))
                    {
                        if (Name.StartsWith(TEXT("Module.Lifetime")))
                            User = TEXT("User.Lifetime");
                        if (Name.StartsWith(TEXT("Module.Uniform Sprite Size")))
                            User = TEXT("User.SpriteSize");
                        if (Name == TEXT("Module.Color"))
                            User = TEXT("User.Color");
                    }
                    if (User.IsNone())
                        continue;
                    const auto Alias = FNiagaraParameterHandle::CreateAliasedModuleParameterHandle(
                        FNiagaraParameterHandle(Input.GetName()), N);
                    auto &Pin =
                        FNiagaraStackGraphUtilities::GetOrCreateStackFunctionInputOverridePin(
                            *N, Alias, Input.GetType(), FGuid(), FGuid());
                    Pin.BreakAllPinLinks(); // Replace the template's optional random-range dynamic
                                            // input.
                    FNiagaraVariable Linked(Input.GetType(), User);
                    TSet<FNiagaraVariableBase> Known;
                    Known.Add(Linked);
                    FNiagaraStackGraphUtilities::SetLinkedParameterValueForFunctionInput(
                        Pin, Linked, Known);
                    UE_LOG(LogTemp, Display, TEXT("VG11 Niagara bound %s / %s -> %s"), *Module,
                           *Name, *User.ToString());
                }
            }
        }
    System->RequestCompile(true);
    System->WaitForCompilationComplete();
    Save(System);
}
} // namespace Visual11Author
#endif
UVisual11AuthorCommandlet::UVisual11AuthorCommandlet()
{
    IsClient =
        true; // Niagara render assets are client content; do not filter them during authoring.
    IsServer = false;
    IsEditor = true;
    LogToConsole = true;
}
int32 UVisual11AuthorCommandlet::Main(const FString &Params)
{
#if WITH_EDITOR
    using namespace Visual11Author;
    if (Params.Contains(TEXT("NiagaraOnly")))
    {
        Niagara(LoadObject<UMaterial>(nullptr, TEXT("/Game/DaoVoTan/Visual11/M_QiSprite")));
        return 0;
    }
    auto *Rig = LoadObject<USkeletalMesh>(
        nullptr, TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple"));
    if (!Rig)
        return 2;
    const auto &Ref = Rig->GetRefSkeleton();
    TArray<FTransform> CS;
    for (int32 I = 0; I < Ref.GetNum(); ++I)
    {
        int32 Parent = Ref.GetParentIndex(I);
        CS.Add(Parent < 0 ? Ref.GetRefBonePose()[I] : Ref.GetRefBonePose()[I] * CS[Parent]);
    }
    auto Bone = [&](const TCHAR *N) { return Ref.FindBoneIndex(N); };
    auto Pos = [&](const TCHAR *N) { return CS[Bone(N)].GetTranslation(); };
    auto *Cloth = Surface(TEXT("M_Robe"), FLinearColor(.035f, .12f, .15f));
    auto *Gold = Surface(TEXT("M_Embroidery"), FLinearColor(.58f, .36f, .10f), .38f, .45f);
    auto *Skin = Surface(TEXT("M_Skin"), FLinearColor(.55f, .31f, .19f), .8f);
    auto *Dark = Surface(TEXT("M_Hair"), FLinearColor(.012f, .018f, .022f), .45f);
    auto *Steel = Surface(TEXT("M_JianSteel"), FLinearColor(.48f, .65f, .68f), .25f, .85f);
    auto *Aura = Surface(TEXT("M_QiSprite"), FLinearColor(.15f, .85f, .65f), .6f, 0, true);
    auto *PortraitBackground =
        Surface(TEXT("M_PortraitBackground"), FLinearColor(.015f, .03f, .038f));
    PortraitBackground->SetShadingModel(MSM_Unlit);
    PortraitBackground->GetEditorOnlyData()->EmissiveColor.Expression =
        PortraitBackground->GetEditorOnlyData()->BaseColor.Expression;
    PortraitBackground->PostEditChange();
    Save(PortraitBackground);
    // The modular face origin sits below Manny's head pivot so the stylized
    // neck remains short without changing the shared animation skeleton.
    FVector Head = Pos(TEXT("head")) - FVector(0, 0, 14);
    for (int32 Variant = 0; Variant < 3; ++Variant)
    {
        FVisualGeometry G;
        G.Ellipsoid(Head + FVector(0, 1, 10),
                    FVector(8.0f + Variant * .7f, 8.8f, 11.7f - Variant * .4f), 0);
        for (float Side : {-1.f, 1.f})
        {
            G.Ellipsoid(Head + FVector(Side * 3.6f, 8.8f, 12), FVector(1.6f, .65f, .7f), 1);
            G.Ellipsoid(Head + FVector(Side * 8.2f, 0, 10), FVector(1.4f, 2.4f, 3), 0);
            G.Tube(Head + FVector(Side * 2.2f, 8.7f, 14),
                   Head + FVector(Side * 5.3f, 8.2f, 14.5f + Variant * .3f), .45f, .3f, 1, 0);
        }
        G.Ellipsoid(Head + FVector(0, 9, 9), FVector(1.3f, 2.2f, 2.4f), 0);
        G.Ellipsoid(Head + FVector(0, 8.8f, 5), FVector(2.2f, .4f, .45f), 1);
        G.Static(FString::Printf(TEXT("SM_Head%d"), Variant), {Skin, Dark},
                 &CS[Bone(TEXT("head"))]);
    }
    for (int32 Variant = 0; Variant < 4; ++Variant)
    {
        FVisualGeometry G;
        G.Ellipsoid(Head + FVector(0, -1, 18), FVector(8.8f, 9.1f, 5.6f), 0);
        if (Variant == 0 || Variant == 2)
            G.Ellipsoid(Head + FVector(0, -1, 25), FVector(4, 4, 4), 0);
        if (Variant >= 1)
            for (float Side : {-1.f, 1.f})
                G.Tube(Head + FVector(Side * 6, -5, 17),
                       Head + FVector(Side * 7, -6, Variant == 3 ? -15 : -3), 2.7f, 1.5f, 0, 0);
        if (Variant == 2)
            G.Tube(Head + FVector(0, -7, 21), Head + FVector(0, -11, -12), 3.8f, 1.2f, 0, 0);
        G.Tube(Head + FVector(-5, 0, 23), Head + FVector(5, 0, 23), .55f, .55f, 1, 0);
        G.Static(FString::Printf(TEXT("SM_Hair%d"), Variant), {Dark, Gold},
                 &CS[Bone(TEXT("head"))]);
    }
    for (int32 Variant = 0; Variant < 3; ++Variant)
    {
        FVisualGeometry G;
        G.Tube(Pos(TEXT("pelvis")) - FVector(0, 0, 6), Pos(TEXT("spine_03")) + FVector(0, 0, 4), 17,
               23 + Variant, 0, Bone(TEXT("pelvis")), Bone(TEXT("spine_03")), .63f);
        G.Ellipsoid(Pos(TEXT("spine_03")) + FVector(0, 0, 9), FVector(22, 14, 20), 0,
                    Bone(TEXT("spine_03")));
        G.Tube(Pos(TEXT("pelvis")) - FVector(0, 0, 5), Pos(TEXT("pelvis")) + FVector(0, 0, 1),
               18.2f, 18.2f, 1, Bone(TEXT("pelvis")), -1, .68f);
        const int32 SkirtStart = G.Points.Num();
        const FVector Hip = Pos(TEXT("pelvis"));
        G.Tube(Hip - FVector(0, 0, 5), Hip - FVector(0, 0, Variant == 1 ? 45 : 57), 21,
               Variant == 2 ? 32 : 28, 0, Bone(TEXT("pelvis")), -1, .8f);
        for (int32 V = SkirtStart; V < G.Points.Num(); ++V)
            if (G.Points[V].Z < Hip.Z - 18)
                G.Bones[V] = Bone(G.Points[V].X < Hip.X ? TEXT("thigh_l") : TEXT("thigh_r"));
        G.Tube(Pos(TEXT("neck_01")) + FVector(-6, 9, 0), Hip + FVector(9, 14, -3), 1.3f, 1.3f, 1,
               Bone(TEXT("spine_03")), Bone(TEXT("pelvis")));
        G.Tube(Pos(TEXT("neck_01")) + FVector(6, 9, 0), Hip + FVector(-9, 14, -3), 1.3f, 1.3f, 1,
               Bone(TEXT("spine_03")), Bone(TEXT("pelvis")));
        for (const TCHAR *Side : {TEXT("l"), TEXT("r")})
        {
            FString S = Side;
            auto N = [&](const TCHAR *Prefix) { return FName(*(FString(Prefix) + S)); };
            auto B = [&](const TCHAR *Prefix) { return Ref.FindBoneIndex(N(Prefix)); };
            auto P0 = [&](const TCHAR *Prefix) { return CS[B(Prefix)].GetTranslation(); };
            G.Tube(P0(TEXT("upperarm_")), P0(TEXT("lowerarm_")), 10 + Variant, 9 + Variant, 0,
                   B(TEXT("upperarm_")), B(TEXT("lowerarm_")));
            G.Ellipsoid(P0(TEXT("upperarm_")), FVector(8 + Variant, 8 + Variant, 8), 0,
                        B(TEXT("upperarm_")));
            G.Tube(P0(TEXT("lowerarm_")), P0(TEXT("hand_")), 9 + Variant, 7.5f + Variant, 0,
                   B(TEXT("lowerarm_")));
            FVector Hand = P0(TEXT("hand_"));
            G.Ellipsoid(Hand, FVector(4.5f, 6, 3.5f), 2, B(TEXT("hand_")));
            G.Tube(P0(TEXT("thigh_")) - FVector(0, 0, 4),
                   P0(TEXT("calf_")) + FVector(0, 0, Variant == 1 ? 4 : -5), 15 + Variant,
                   19 + Variant, 0, B(TEXT("thigh_")), B(TEXT("calf_")), .8f);
            G.Tube(P0(TEXT("calf_")) + FVector(0, 0, 1), P0(TEXT("foot_")), 8, 6, 3,
                   B(TEXT("calf_")));
            G.Ellipsoid(P0(TEXT("foot_")) + FVector(0, 6, 1), FVector(6, 13, 6), 3,
                        B(TEXT("foot_")));
        }
        G.Tube(Pos(TEXT("neck_01")) - FVector(0, 0, 3), Head + FVector(0, 0, 3), 6, 5.5f, 2,
               Bone(TEXT("neck_01")), Bone(TEXT("head")));
        G.Skinned(FString::Printf(TEXT("SK_Robe%d"), Variant), Rig, {Cloth, Gold, Skin, Dark});
    }
    for (int32 I = 0; I < 2; ++I)
    {
        FVisualGeometry G;
        if (I == 0)
        {
            G.Ellipsoid(FVector(0, 0, -7), FVector(5, 3, 7), 0);
            G.Tube(FVector(0, 0, 0), FVector(0, 0, 6), 1, 1, 1, 0);
        }
        else
        {
            for (int32 J = 0; J < 9; ++J)
            {
                float A = 2 * PI * J / 9;
                G.Ellipsoid(FVector(FMath::Cos(A) * 8, FMath::Sin(A) * 8, 0), FVector(1.8f), 0);
            }
        }
        G.Static(FString::Printf(TEXT("SM_Accessory%d"), I), {Gold, Dark});
    }
    FVisualGeometry Jian;
    int32 A = Jian.Vertex(FVector(-2, 0, 15)), B = Jian.Vertex(FVector(0, 1.2f, 15)),
          C = Jian.Vertex(FVector(2, 0, 15)), D = Jian.Vertex(FVector(0, -1.2f, 15)),
          Tip = Jian.Vertex(FVector(0, 0, 101));
    for (auto F : {FIntVector(A, B, Tip), FIntVector(B, C, Tip), FIntVector(C, D, Tip),
                   FIntVector(D, A, Tip)})
        Jian.Tri(F.X, F.Y, F.Z, 0);
    Jian.Tube(FVector(0, 0, -5), FVector(0, 0, 14), 1.6f, 1.6f, 2, 0);
    Jian.Tube(FVector(-10, 0, 15), FVector(10, 0, 15), 1.6f, 1.6f, 1, 0);
    Jian.Ellipsoid(FVector(0, 0, -7), FVector(2.6f, 2, 3), 1);
    Jian.Static(TEXT("SM_Jian"), {Steel, Gold, Dark});
    FVisualGeometry Rock;
    Rock.Ellipsoid(FVector::ZeroVector, FVector(48, 40, 53), 0, 0, 14, 10);
    for (auto &V : Rock.Points)
        V *= 1 + .16f * FMath::Sin(V.X * .1f + V.Z * .13f) * FMath::Cos(V.Y * .17f);
    Rock.Static(TEXT("SM_ScholarRock"),
                {LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/DaoVoTan/Materials/M_Rock"))});
    FVisualGeometry Canopy;
    Canopy.Ellipsoid(FVector(-25, 0, 0), FVector(38, 42, 24), 0);
    Canopy.Ellipsoid(FVector(25, 5, 12), FVector(46, 33, 22), 0);
    Canopy.Ellipsoid(FVector(0, -20, 32), FVector(35, 38, 23), 0);
    Canopy.Static(TEXT("SM_Canopy"), {LoadObject<UMaterialInterface>(
                                         nullptr, TEXT("/Game/DaoVoTan/Materials/M_Leaf"))});
    const TCHAR *Recipes[] = {TEXT("/Game/DaoVoTan/Data/DA_Player"),
                              TEXT("/Game/DaoVoTan/Data/DA_NPC_A"),
                              TEXT("/Game/DaoVoTan/Data/DA_NPC_B")};
    for (int32 I = 0; I < 3; ++I)
    {
        auto *R = LoadObject<UPresentationRecipe>(nullptr, Recipes[I]);
        check(R);
        R->Body = Rig;
        R->HeadVariant = I;
        R->HairVariant = I;
        R->GarmentVariant = I;
        R->AccessoryVariant = I % 2;
        R->bAccessory = true;
        R->FactionColor = R->TrimColor;
        Save(R);
    }
    SwordAnimation(Rig);
    Niagara(Aura);
    UWorld *W = UEditorLoadingAndSavingUtils::LoadMap(TEXT("/Game/DaoVoTan/Maps/StillwaterValley"));
    if (!W)
        return 3;
    for (TActorIterator<AValleyStage> I(W); I; ++I)
        I->RerunConstructionScripts();
    if (!UEditorLoadingAndSavingUtils::SaveMap(W, TEXT("/Game/DaoVoTan/Maps/StillwaterValley")))
        return 4;
    UE_LOG(LogTemp, Display,
           TEXT("VG11 authoring complete: original modular wardrobe/head/hair/jian, shared rig, "
                "sword animation, one Niagara system."));
    return 0;
#else
    return 1;
#endif
}
