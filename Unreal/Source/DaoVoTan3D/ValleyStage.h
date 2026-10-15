#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ValleyStage.generated.h"
UCLASS()
class DAOVOTAN3D_API AValleyStage : public AActor
{
    GENERATED_BODY()
  public:
    AValleyStage();
    virtual void OnConstruction(const FTransform &T) override;

  private:
    UPROPERTY() TObjectPtr<class UHierarchicalInstancedStaticMeshComponent> Rocks;
    UPROPERTY() TObjectPtr<class UHierarchicalInstancedStaticMeshComponent> Trunks;
    UPROPERTY() TObjectPtr<class UHierarchicalInstancedStaticMeshComponent> Leaves;
    UPROPERTY() TObjectPtr<class UHierarchicalInstancedStaticMeshComponent> Stone;
    UPROPERTY() TObjectPtr<class UHierarchicalInstancedStaticMeshComponent> Wood;
    UPROPERTY() TObjectPtr<class UHierarchicalInstancedStaticMeshComponent> Gold;
};
