#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GatePlayerController.generated.h"
UCLASS()
class DAOVOTAN3D_API AGatePlayerController : public APlayerController
{
    GENERATED_BODY()
  public:
    AGatePlayerController();
    UPROPERTY(VisibleAnywhere) TObjectPtr<class UCultivationCommandAdapter> CultivationCommands;
    virtual bool InputKey(const FInputKeyEventArgs &Params) override;

  private:
    void ToggleExperimentalAppearance();

  protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
};
