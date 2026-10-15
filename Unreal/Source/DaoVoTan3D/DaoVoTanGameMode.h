#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DaoVoTanGameMode.generated.h"

UCLASS()
class DAOVOTAN3D_API ADaoVoTanGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ADaoVoTanGameMode();

protected:
    virtual void BeginPlay() override;

private:
    void EnsurePrototypeWorld();
    void EnsurePlayerPawn();
};
