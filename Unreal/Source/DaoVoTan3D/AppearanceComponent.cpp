#include "AppearanceComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Character.h"
#include "Materials/MaterialInstanceDynamic.h"
void UAppearanceComponent::Apply(UPresentationRecipe *R)
{
    auto *C = Cast<ACharacter>(GetOwner());
    if (!C || !R)
        return;
    for (auto M : Modules)
        if (M)
            M->DestroyComponent();
    Modules.Empty();
    if (Garment)
        Garment->DestroyComponent();
    Recipe = R;
    C->GetMesh()->SetSkeletalMeshAsset(LoadObject<USkeletalMesh>(
        nullptr, TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple")));
    C->GetMesh()->SetVisibility(false, false);
    C->GetMesh()->VisibilityBasedAnimTickOption =
        EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
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
}
