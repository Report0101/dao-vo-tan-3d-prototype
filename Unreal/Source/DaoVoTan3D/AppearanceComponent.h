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

  private:
    UPROPERTY(Transient) TArray<TObjectPtr<UStaticMeshComponent>> Modules;
};
