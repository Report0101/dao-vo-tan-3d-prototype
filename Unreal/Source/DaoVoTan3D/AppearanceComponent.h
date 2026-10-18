#pragma once
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "PresentationRecipe.h"
#include "AppearanceComponent.generated.h"
UCLASS(ClassGroup = Presentation, meta = (BlueprintSpawnableComponent))
class DAOVOTAN3D_API UAppearanceComponent : public UActorComponent
{
    GENERATED_BODY()
  public:
    UFUNCTION(BlueprintCallable) void Apply(UPresentationRecipe *InRecipe);
    UPROPERTY(Transient) TObjectPtr<UPresentationRecipe> Recipe;
    UPROPERTY(Transient) TObjectPtr<class USkeletalMeshComponent> Garment;
    int32 ModuleCount() const
    {
        return Modules.Num() + SkinnedModules.Num();
    }

    /** Local editor PIE selection only; never changes actor Recipe or simulation state. */
    bool ToggleExperimentalPreview();
    bool IsExperimentalPreview() const { return bExperimentalPreview; }
    const TArray<TObjectPtr<class USkeletalMeshComponent>>& GetSkinnedModules() const
    { return SkinnedModules; }

  private:
    bool RebuildVisuals(UPresentationRecipe *VisualRecipe);
    bool bExperimentalPreview = false;
    UPROPERTY(Transient) TObjectPtr<UPresentationRecipe> ExperimentalRecipe;
    UPROPERTY(Transient) TArray<TObjectPtr<class USkeletalMeshComponent>> SkinnedModules;
    UPROPERTY(Transient) TArray<TObjectPtr<UStaticMeshComponent>> Modules;
};
