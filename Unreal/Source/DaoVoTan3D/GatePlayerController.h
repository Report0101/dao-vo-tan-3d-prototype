#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GatePlayerController.generated.h"
UCLASS()
class DAOVOTAN3D_API AGatePlayerController : public APlayerController
{
    GENERATED_BODY()
  protected:
    virtual void BeginPlay() override;
};
