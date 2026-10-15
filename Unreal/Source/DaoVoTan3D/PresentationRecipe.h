#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PresentationRecipe.generated.h"
USTRUCT(BlueprintType)
struct FAuraStyle
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor Color = FLinearColor(0.12f, 0.8f, 0.65f);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "5")) float Intensity = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "30", ClampMax = "250")) float Radius = 85;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "8", ClampMax = "96")) int32 Density = 32;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "5")) float PulseSpeed = 1;
};
UCLASS(BlueprintType)
class DAOVOTAN3D_API UPresentationRecipe : public UDataAsset
{
    GENERATED_BODY()
  public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText DisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText RealmLabel;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<USkeletalMesh> Body;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLinearColor GarmentColor = FLinearColor(0.06f, 0.22f, 0.24f);
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLinearColor TrimColor = FLinearColor(0.8f, 0.57f, 0.17f);
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bHeadwear = false;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bAccessory = false;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bSword = true;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FAuraStyle Aura;
};
