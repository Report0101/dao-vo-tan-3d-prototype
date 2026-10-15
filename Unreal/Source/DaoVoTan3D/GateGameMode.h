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
    virtual void BeginPlay() override;
};
