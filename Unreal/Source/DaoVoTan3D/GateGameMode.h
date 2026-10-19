#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GateGameMode.generated.h"
UCLASS()
class DAOVOTAN3D_API AGateGameMode : public AGameModeBase
{
    GENERATED_BODY()
  public:
    AGateGameMode();
    UPROPERTY(VisibleAnywhere) TObjectPtr<class USnapshotPresentationAdapter> SnapshotAdapter;
    virtual void BeginPlay() override;
    void RetireOfflinePreviewActors();
  private:
    UPROPERTY() TArray<TObjectPtr<class AGateCharacter>> OfflinePreviewActors;
};
