#pragma once
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"
#include "PresentationRecipe.h"
#include "AuraComponent.generated.h"
UCLASS(ClassGroup = Presentation, meta = (BlueprintSpawnableComponent))
class DAOVOTAN3D_API UAuraComponent : public USceneComponent
{
    GENERATED_BODY()
  public:
    UAuraComponent();
    void Configure(const FAuraStyle &InStyle);
    void SetEmitting(bool bEnabled);
    virtual void BeginPlay() override;
    virtual void TickComponent(float Dt, ELevelTick Tick, FActorComponentTickFunction *Fn) override;
    bool IsEmitting() const
    {
        return bEmitting;
    }
    class UNiagaraComponent *NiagaraInstance() const
    {
        return Motes;
    }
    const FAuraStyle &GetStyle() const
    {
        return Style;
    }

  private:
    UPROPERTY() TObjectPtr<class UNiagaraComponent> Motes;
    UPROPERTY() TObjectPtr<class UMaterialInstanceDynamic> Material;
    FAuraStyle Style;
    bool bEmitting = false;
    float Age = 0;
};
