#include "RetopoGate1ValidateCommandlet.h"
#if WITH_EDITOR
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Engine/SkeletalMesh.h"
#include "Rendering/SkeletalMeshModel.h"
#include "Rendering/SkeletalMeshLODModel.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "GateCharacter.h"
namespace
{
TSharedPtr<FJsonValue> V(FVector P)
{
    return MakeShared<FJsonValueArray>(TArray<TSharedPtr<FJsonValue>>{MakeShared<FJsonValueNumber>(P.X),
                                                                      MakeShared<FJsonValueNumber>(P.Y),
                                                                      MakeShared<FJsonValueNumber>(P.Z)});
}
TArray<FTransform> Globals(const FReferenceSkeleton &R)
{
    TArray<FTransform> G;
    for (int32 I = 0; I < R.GetNum(); ++I)
    {
        const int32 P = R.GetParentIndex(I);
        G.Add(P < 0 ? R.GetRefBonePose()[I] : R.GetRefBonePose()[I] * G[P]);
    }
    return G;
}
bool Write(const TSharedPtr<FJsonObject> &D, const FString &Path)
{
    FString S;
    auto W = TJsonWriterFactory<>::Create(&S);
    return FJsonSerializer::Serialize(D.ToSharedRef(), W) && FFileHelper::SaveStringToFile(S, *Path);
}
} // namespace
#endif
URetopoGate1ValidateCommandlet::URetopoGate1ValidateCommandlet()
{
    IsClient = false;
    IsServer = false;
    IsEditor = true;
    LogToConsole = true;
}
int32 URetopoGate1ValidateCommandlet::Main(const FString &Params)
{
#if WITH_EDITOR
    FString Input, Output;
    if (!FParse::Value(*Params, TEXT("Fixture="), Input) ||
        !FParse::Value(*Params, TEXT("Evidence="), Output))
        return 2;
    FString Text;
    if (!FFileHelper::LoadFileToString(Text, *Input))
        return 3;
    TSharedPtr<FJsonObject> Config;
    auto Reader = TJsonReaderFactory<>::Create(Text);
    if (!FJsonSerializer::Deserialize(Reader, Config))
        return 4;
    auto *Manny = LoadObject<USkeletalMesh>(nullptr, *Config->GetStringField(TEXT("referenceMesh")));
    if (!Manny)
        return 5;
    const auto &MR = Manny->GetRefSkeleton();
    const auto MG = Globals(MR);
    const FString Boundary = TEXT("/Game/DaoVoTan/Experimental/PlayerV1/RetopoGate1/");
    IFileManager::Get().MakeDirectory(*Output, true);
    TArray<USkeletalMesh *> Meshes;
    TArray<TSharedPtr<FJsonValue>> Modules;
    bool Fatal = false;
    for (const auto &A : Config->GetArrayField(TEXT("modules")))
    {
        const FString Path = A->AsString();
        if (!Path.StartsWith(Boundary))
            return 6;
        auto *M = LoadObject<USkeletalMesh>(nullptr, *Path);
        if (!M || M->GetSkeleton() != Manny->GetSkeleton())
            return 7;
        Meshes.Add(M);
        const auto &R = M->GetRefSkeleton();
        const auto G = Globals(R);
        double MaxP = 0, MaxAngle = 0, MaxScale = 0;
        TArray<TSharedPtr<FJsonValue>> Unknown;
        for (int32 I = 0; I < R.GetNum(); ++I)
        {
            const int32 J = MR.FindBoneIndex(R.GetBoneName(I));
            if (J < 0)
            {
                Fatal = true;
                Unknown.Add(MakeShared<FJsonValueString>(R.GetBoneName(I).ToString()));
                continue;
            }
            MaxP = FMath::Max(MaxP, FVector::Dist(G[I].GetTranslation(), MG[J].GetTranslation()));
            MaxAngle = FMath::Max(
                MaxAngle, FMath::RadiansToDegrees(G[I].GetRotation().AngularDistance(MG[J].GetRotation())));
            MaxScale = FMath::Max(MaxScale, (G[I].GetScale3D() - MG[J].GetScale3D()).GetAbsMax());
        }
        auto D = MakeShared<FJsonObject>();
        D->SetStringField(TEXT("mesh"), Path);
        D->SetNumberField(TEXT("bones"), R.GetNum());
        D->SetArrayField(TEXT("unknownReferenceBones"), Unknown);
        D->SetNumberField(TEXT("maxReferencePositionErrorCm"), MaxP);
        D->SetNumberField(TEXT("maxReferenceRotationErrorDegrees"), MaxAngle);
        D->SetNumberField(TEXT("maxReferenceScaleError"), MaxScale);
        const auto &L = M->GetImportedModel()->LODModels[0];
        int32 Vertices = 0, Triangles = 0, Unweighted = 0;
        TArray<TSharedPtr<FJsonValue>> Sections;
        for (const auto &S : L.Sections)
        {
            Vertices += S.SoftVertices.Num();
            Triangles += S.NumTriangles;
            auto Q = MakeShared<FJsonObject>();
            Q->SetNumberField(TEXT("vertices"), S.SoftVertices.Num());
            Q->SetNumberField(TEXT("triangles"), S.NumTriangles);
            Q->SetNumberField(TEXT("materialIndex"), S.MaterialIndex);
            Sections.Add(MakeShared<FJsonValueObject>(Q));
            for (const auto &X : S.SoftVertices)
            {
                uint32 Sum = 0;
                for (int32 K = 0; K < MAX_TOTAL_INFLUENCES; ++K)
                    Sum += X.InfluenceWeights[K];
                if (Sum == 0)
                    ++Unweighted;
            }
        }
        D->SetNumberField(TEXT("importedVertices"), Vertices);
        D->SetNumberField(TEXT("triangles"), Triangles);
        D->SetNumberField(TEXT("UVChannels"), L.NumTexCoords);
        D->SetNumberField(TEXT("materialSlots"), M->GetMaterials().Num());
        D->SetArrayField(TEXT("sections"), Sections);
        D->SetNumberField(TEXT("unweightedVertices"), Unweighted);
        D->SetBoolField(TEXT("referenceCompatible"),
                        Unknown.IsEmpty() && MaxP < .05 && MaxAngle < .05 && MaxScale < .001);
        Fatal |= MaxP >= .05 || MaxAngle >= .05 || MaxScale >= .001 || Unweighted > 0;
        Modules.Add(MakeShared<FJsonValueObject>(D));
    }
    auto Result = MakeShared<FJsonObject>();
    Result->SetArrayField(TEXT("modules"), Modules);
    TArray<TSharedPtr<FJsonValue>> Clips;
    for (const auto &E : Config->GetObjectField(TEXT("clips"))->Values)
    {
        auto *Clip = LoadObject<UAnimSequence>(nullptr, *E.Value->AsString());
        if (!Clip || Clip->GetSkeleton() != Manny->GetSkeleton())
            return 8;
        auto C = MakeShared<FJsonObject>();
        C->SetStringField(TEXT("name"), E.Key);
        C->SetStringField(TEXT("clip"), E.Value->AsString());
        TArray<TSharedPtr<FJsonValue>> Frames;
        for (int32 F = 0; F <= 6; ++F)
        {
            const double Fraction = F / 6.;
            const double T = Clip->GetPlayLength() * Fraction;
            const auto &SR = Clip->GetSkeleton()->GetReferenceSkeleton();
            TArray<FTransform> Pose;
            for (int32 I = 0; I < SR.GetNum(); ++I)
            {
                FTransform Local = SR.GetRefBonePose()[I];
                Clip->GetBoneTransform(Local, FSkeletonPoseBoneIndex(I), FAnimExtractContext(T), true);
                const int32 P = SR.GetParentIndex(I);
                Pose.Add(P < 0 ? Local : Local * Pose[P]);
            }
            FString OBJ;
            int32 Base = 1;
            FBox Bounds(ForceInit);
            int32 NonFinite = 0;
            double WorstStretch = 0;
            TArray<TSharedPtr<FJsonValue>> PerModule;
            for (auto *M : Meshes)
            {
                const auto &R = M->GetRefSkeleton();
                const auto Rest = Globals(R);
                const auto &L = M->GetImportedModel()->LODModels[0];
                TArray<FVector> Verts;
                Verts.SetNumZeroed(L.NumVertices);
                for (const auto &S : L.Sections)
                    for (int32 I = 0; I < S.SoftVertices.Num(); ++I)
                    {
                        const auto &X = S.SoftVertices[I];
                        FVector P = FVector::ZeroVector;
                        double Total = 0;
                        for (int32 K = 0; K < MAX_TOTAL_INFLUENCES; ++K)
                        {
                            if (!X.InfluenceWeights[K])
                                continue;
                            const int32 B = S.BoneMap[X.InfluenceBones[K]];
                            const int32 J = SR.FindBoneIndex(R.GetBoneName(B));
                            if (J < 0)
                            {
                                Fatal = true;
                                continue;
                            }
                            const double W = X.InfluenceWeights[K] / 65535.;
                            P += Pose[J].TransformPosition(
                                     Rest[B].InverseTransformPosition(FVector(X.Position))) *
                                 W;
                            Total += W;
                        }
                        if (Total > 0)
                            P /= Total;
                        if (P.ContainsNaN())
                            ++NonFinite;
                        Bounds += P;
                        Verts[S.BaseVertexIndex + I] = P;
                    }
                double Stretch = 0;
                int32 Over3 = 0;
                for (const auto &S : L.Sections)
                    for (uint32 I = 0; I < S.NumTriangles; ++I)
                        for (int32 K = 0; K < 3; ++K)
                        {
                            const uint32 A = L.IndexBuffer[S.BaseIndex + I * 3 + K],
                                         B = L.IndexBuffer[S.BaseIndex + I * 3 + (K + 1) % 3];
                            // Index-to-source lookup respects duplicated section vertices.
                            FVector PA, PB;
                            for (const auto &Q : L.Sections)
                            {
                                if (A >= Q.BaseVertexIndex && A < Q.BaseVertexIndex + Q.NumVertices)
                                    PA = FVector(Q.SoftVertices[A - Q.BaseVertexIndex].Position);
                                if (B >= Q.BaseVertexIndex && B < Q.BaseVertexIndex + Q.NumVertices)
                                    PB = FVector(Q.SoftVertices[B - Q.BaseVertexIndex].Position);
                            }
                            const double Ratio =
                                FVector::Dist(Verts[A], Verts[B]) / FMath::Max(1e-5, FVector::Dist(PA, PB));
                            Stretch = FMath::Max(Stretch, Ratio);
                            if (Ratio > 3)
                                ++Over3;
                        }
                auto D = MakeShared<FJsonObject>();
                D->SetStringField(TEXT("module"), M->GetName());
                D->SetNumberField(TEXT("maxEdgeStretch"), Stretch);
                D->SetNumberField(TEXT("triangleEdgesOver3x"), Over3);
                PerModule.Add(MakeShared<FJsonValueObject>(D));
                WorstStretch = FMath::Max(WorstStretch, Stretch);
                if (F == 3)
                {
                    OBJ += TEXT("o ") + M->GetName() + TEXT("\n");
                    for (FVector P : Verts)
                        OBJ +=
                            FString::Printf(TEXT("v %.6f %.6f %.6f\n"), P.X / 100., -P.Y / 100., P.Z / 100.);
                    for (const auto &S : L.Sections)
                        for (uint32 I = 0; I < S.NumTriangles; ++I)
                            OBJ += FString::Printf(TEXT("f %d %d %d\n"),
                                                   Base + L.IndexBuffer[S.BaseIndex + I * 3],
                                                   Base + L.IndexBuffer[S.BaseIndex + I * 3 + 1],
                                                   Base + L.IndexBuffer[S.BaseIndex + I * 3 + 2]);
                    Base += Verts.Num();
                }
            }
            auto D = MakeShared<FJsonObject>();
            D->SetNumberField(TEXT("fraction"), Fraction);
            D->SetNumberField(TEXT("timeSeconds"), T);
            D->SetField(TEXT("boundsMinCm"), V(Bounds.Min));
            D->SetField(TEXT("boundsMaxCm"), V(Bounds.Max));
            D->SetNumberField(TEXT("nonFiniteVertices"), NonFinite);
            D->SetNumberField(TEXT("maxEdgeStretch"), WorstStretch);
            D->SetArrayField(TEXT("modules"), PerModule);
            Frames.Add(MakeShared<FJsonValueObject>(D));
            Fatal |= NonFinite > 0;
            if (F == 3)
            {
                const int32 Hand = SR.FindBoneIndex(TEXT("hand_r"));
                const FTransform Sword = AGateCharacter::JianGripTransform() * Pose[Hand];
                D->SetField(TEXT("swordRootCm"), V(Sword.GetTranslation()));
                D->SetField(TEXT("swordX"), V(Sword.GetUnitAxis(EAxis::X)));
                D->SetField(TEXT("swordY"), V(Sword.GetUnitAxis(EAxis::Y)));
                D->SetField(TEXT("swordZ"), V(Sword.GetUnitAxis(EAxis::Z)));
                if (!FFileHelper::SaveStringToFile(OBJ, *(Output / TEXT("pose_") + E.Key + TEXT(".obj"))))
                    return 9;
            }
        }
        C->SetArrayField(TEXT("frames"), Frames);
        Clips.Add(MakeShared<FJsonValueObject>(C));
    }
    Result->SetArrayField(TEXT("clips"), Clips);
    Result->SetBoolField(TEXT("structuralChecksPassed"), !Fatal);
    Result->SetStringField(
        TEXT("method"),
        TEXT("Read-only UE imported LOD0 CPU linear-blend skinning, seven times per accepted clip. No "
             "runtime graph or gameplay edits. Visual clipping/grip assessment remains separate."));
    if (!Write(Result, Output / TEXT("engine-deformation.json")))
        return 10;
    return Fatal ? 11 : 0;
#else
    return 1;
#endif
}
