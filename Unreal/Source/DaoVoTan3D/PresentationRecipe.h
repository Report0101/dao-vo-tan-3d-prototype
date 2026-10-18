#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PresentationRecipe.generated.h"
class UMaterialInterface;
USTRUCT(BlueprintType)
struct FAuraStyle
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor Color = FLinearColor(0.12f, 0.8f, 0.65f);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "5"))
    float Intensity = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "30", ClampMax = "250"))
    float Radius = 85;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "8", ClampMax = "96"))
    int32 Density = 32;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "5"))
    float PulseSpeed = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float VerticalMotion = 18;
};
UCLASS(BlueprintType)
class DAOVOTAN3D_API UPresentationRecipe : public UDataAsset
{
    GENERATED_BODY()
  public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText DisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText RealmLabel;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<USkeletalMesh> Body;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 HeadVariant = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 HairVariant = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 GarmentVariant = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 AccessoryVariant = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FLinearColor SkinColor = FLinearColor(.55f, .31f, .19f);
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FLinearColor HairColor = FLinearColor(.016f, .022f, .026f);
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FLinearColor FactionColor = FLinearColor(.58f, .36f, .10f);
    /** Optional modular presentation; empty retains the accepted variant-based appearance. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<TSoftObjectPtr<USkeletalMesh>> SkeletalModules;
    /** Slot overrides are component-only; source meshes remain unchanged. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TMap<FName, TSoftObjectPtr<UMaterialInterface>> SkeletalSlotMaterials;
    void ApplyAppearanceSeed(const FString &Seed);
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FLinearColor GarmentColor = FLinearColor(0.06f, 0.22f, 0.24f);
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FLinearColor TrimColor = FLinearColor(0.8f, 0.57f, 0.17f);
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bHeadwear = false;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bAccessory = false;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bSword = true;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FAuraStyle Aura;
};
