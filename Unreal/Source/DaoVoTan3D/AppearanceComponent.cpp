#include "AppearanceComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
void UAppearanceComponent::Apply(UPresentationRecipe *R)
{
    if (!R) return;
    Recipe = R; // Keep the latest seeded/current recipe for exact restoration after resync.
    RebuildVisuals(bExperimentalPreview ? ExperimentalRecipe.Get() : R);
}
bool UAppearanceComponent::ToggleExperimentalPreview()
{
#if WITH_EDITOR
    auto *C = Cast<ACharacter>(GetOwner());
    if (!C || !GetWorld() || GetWorld()->WorldType != EWorldType::PIE ||
        !C->IsPlayerControlled() || !C->IsLocallyControlled() || !Recipe)
        return false;
    if (!bExperimentalPreview && !ExperimentalRecipe)
        ExperimentalRecipe = LoadObject<UPresentationRecipe>(nullptr,
            TEXT("/Game/DaoVoTan/Experimental/PlayerV1/DA_PlayerV1Experimental"));
    auto *Next = bExperimentalPreview ? Recipe.Get() : ExperimentalRecipe.Get();
    if (!Next || (!bExperimentalPreview && Next->SkeletalModules.IsEmpty()) || !RebuildVisuals(Next))
    {
        UE_LOG(LogTemp, Warning, TEXT("PLAYER_V1_PREVIEW visual assets unavailable; selection unchanged"));
        return false;
    }
    bExperimentalPreview = !bExperimentalPreview;
    UE_LOG(LogTemp, Display, TEXT("PLAYER_V1_PREVIEW owner=%s state=%s"), *GetOwner()->GetName(),
        bExperimentalPreview ? TEXT("PLAYER V1 EXPERIMENTAL") : TEXT("CURRENT PLAYER"));
    return true;
#else
    return false;
#endif
}
bool UAppearanceComponent::RebuildVisuals(UPresentationRecipe *R)
{
    auto *C = Cast<ACharacter>(GetOwner());
    if (!C || !R) return false;
    auto *Leader = LoadObject<USkeletalMesh>(nullptr,
        TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple"));
    if (!Leader) return false;
    // Resolve and validate the complete replacement before touching the visible appearance.
    TArray<USkeletalMesh *> LoadedModules;
    for (const auto &Asset : R->SkeletalModules)
    {
        auto *Mesh = Asset.LoadSynchronous();
        if (!Mesh || Mesh->GetSkeleton() != Leader->GetSkeleton()) return false;
        LoadedModules.Add(Mesh);
    }
    TMap<FName, UMaterialInterface *> LoadedMaterials;
    for (const auto &Entry : R->SkeletalSlotMaterials)
    {
        auto *Material = Entry.Value.LoadSynchronous();
        if (!Material) return false;
        LoadedMaterials.Add(Entry.Key, Material);
    }
    auto Remove = [&](UActorComponent *M)
    {
        if (M) { C->RemoveInstanceComponent(M); M->DestroyComponent(); }
    };
    for (auto M : Modules) Remove(M);
    Modules.Empty();
    for (auto M : SkinnedModules) Remove(M);
    SkinnedModules.Empty();
    Remove(Garment);
    Garment = nullptr;
    // The hidden Manny remains the single animation/IK leader; do not reset its instance.
    if (C->GetMesh()->GetSkeletalMeshAsset() != Leader)
        C->GetMesh()->SetSkeletalMeshAsset(Leader);
    C->GetMesh()->SetVisibility(false, false);
    C->GetMesh()->VisibilityBasedAnimTickOption =
        EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
    if (!LoadedModules.IsEmpty())
    {
        for (auto *Mesh : LoadedModules)
        {
            auto *Part = NewObject<USkeletalMeshComponent>(C);
            C->AddInstanceComponent(Part);
            Part->SetSkeletalMeshAsset(Mesh);
            for (int32 I = 0; I < Mesh->GetMaterials().Num(); ++I)
                if (auto **Material = LoadedMaterials.Find(Mesh->GetMaterials()[I].MaterialSlotName))
                    Part->SetMaterial(I, *Material);
            Part->SetupAttachment(C->GetMesh());
            Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Part->SetGenerateOverlapEvents(false);
            Part->SetCanEverAffectNavigation(false);
            Part->SetLightingChannels(true, true, false);
            Part->RegisterComponent();
            Part->SetLeaderPoseComponent(C->GetMesh());
            SkinnedModules.Add(Part);
        }
        return true;
    }
    auto Tint = [](UMeshComponent *M, int32 Slot, const TCHAR *Path, FLinearColor Color)
    {
        auto *MID =
            UMaterialInstanceDynamic::Create(LoadObject<UMaterialInterface>(nullptr, Path), M);
        MID->SetVectorParameterValue(TEXT("Tint"), Color);
        M->SetMaterial(Slot, MID);
    };
    Garment = NewObject<USkeletalMeshComponent>(C);
    C->AddInstanceComponent(Garment);
    Garment->SetSkeletalMeshAsset(LoadObject<USkeletalMesh>(
        nullptr, *FString::Printf(TEXT("/Game/DaoVoTan/Visual11/SK_Robe%d"),
                                  FMath::Clamp(R->GarmentVariant, 0, 2))));
    Garment->SetupAttachment(C->GetMesh());
    Garment->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Garment->RegisterComponent();
    Garment->SetLeaderPoseComponent(C->GetMesh());
    Garment->SetLightingChannels(true, true, false);
    Tint(Garment, 0, TEXT("/Game/DaoVoTan/Visual11/M_Robe"), R->GarmentColor);
    Tint(Garment, 1, TEXT("/Game/DaoVoTan/Visual11/M_Embroidery"), R->FactionColor);
    Tint(Garment, 2, TEXT("/Game/DaoVoTan/Visual11/M_Skin"), R->SkinColor);
    auto Add = [&](const FString &Path, FName Bone, FVector Offset)
    {
        auto *M = NewObject<UStaticMeshComponent>(C);
        C->AddInstanceComponent(M);
        M->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, *Path));
        M->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        M->SetupAttachment(C->GetMesh(), Bone);
        M->SetRelativeLocation(Offset);
        M->SetLightingChannels(true, true, false);
        M->RegisterComponent();
        Modules.Add(M);
        return M;
    };
    auto *Head = Add(FString::Printf(TEXT("/Game/DaoVoTan/Visual11/SM_Head%d"),
                                     FMath::Clamp(R->HeadVariant, 0, 2)),
                     TEXT("head"), FVector::ZeroVector);
    Tint(Head, 0, TEXT("/Game/DaoVoTan/Visual11/M_Skin"), R->SkinColor);
    auto *Hair = Add(FString::Printf(TEXT("/Game/DaoVoTan/Visual11/SM_Hair%d"),
                                     FMath::Clamp(R->HairVariant, 0, 3)),
                     TEXT("head"), FVector::ZeroVector);
    Tint(Hair, 0, TEXT("/Game/DaoVoTan/Visual11/M_Hair"), R->HairColor);
    Tint(Hair, 1, TEXT("/Game/DaoVoTan/Visual11/M_Embroidery"), R->FactionColor);
    if (R->bAccessory)
        Add(FString::Printf(TEXT("/Game/DaoVoTan/Visual11/SM_Accessory%d"),
                            FMath::Clamp(R->AccessoryVariant, 0, 1)),
            TEXT("pelvis"), FVector(0, 22, -8));
    return true;
}
