#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GateHUD.generated.h"
UCLASS()
class DAOVOTAN3D_API AGateHUD : public AHUD
{
  GENERATED_BODY() public : virtual void DrawHUD() override;
};
