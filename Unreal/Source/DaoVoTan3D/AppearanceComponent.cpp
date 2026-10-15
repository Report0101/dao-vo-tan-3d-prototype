#include "AppearanceComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
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
    Recipe = R;
    C->GetMesh()->SetSkeletalMeshAsset(R->Body.LoadSynchronous());
    auto Add = [&](const TCHAR *Shape, FName Bone, FVector Offset, FVector Scale, FLinearColor Color)
    {
        auto *M = NewObject<UStaticMeshComponent>(C);
        C->AddInstanceComponent(M);
        M->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, Shape));
        M->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        M->AttachToComponent(C->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, Bone);
        M->SetRelativeLocation(Offset);
        M->SetRelativeScale3D(Scale);
        M->RegisterComponent();
        auto *Mat = UMaterialInstanceDynamic::Create(
            LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/DaoVoTan/Materials/M_Color")), M);
        Mat->SetVectorParameterValue(TEXT("Tint"), Color);
        M->SetMaterial(0, Mat);
        Modules.Add(M);
    };
    // Rigid garment/accessory proof on a shared rig, not a final skinned wardrobe.
    Add(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"), TEXT("pelvis"), FVector(0, 0, 0),
        FVector(.44f, .44f, .46f), R->GarmentColor);
    Add(TEXT("/Engine/BasicShapes/Sphere.Sphere"), TEXT("spine_03"), FVector(0, 0, 0),
        FVector(.45f, .31f, .4f), R->GarmentColor);
    if (R->bHeadwear)
        Add(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"), TEXT("head"), FVector(0, 0, 13),
            FVector(.52f, .52f, .06f), R->TrimColor);
    if (R->bAccessory)
        Add(TEXT("/Engine/BasicShapes/Sphere.Sphere"), TEXT("pelvis"), FVector(0, 28, 0),
            FVector(.15f, .15f, .22f), R->TrimColor);
}
