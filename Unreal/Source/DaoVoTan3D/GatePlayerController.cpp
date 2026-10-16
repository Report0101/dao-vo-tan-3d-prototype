#include "GatePlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "CultivationCommandAdapter.h"
#include "EnhancedInputComponent.h"
AGatePlayerController::AGatePlayerController()
{
    CultivationCommands = CreateDefaultSubobject<UCultivationCommandAdapter>(TEXT("CultivationCommands"));
}
void AGatePlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    CultivationCommands->BindInputs(Cast<UEnhancedInputComponent>(InputComponent));
}
void AGatePlayerController::BeginPlay()
{
    Super::BeginPlay();
    SetInputMode(FInputModeGameOnly());
    bShowMouseCursor = false;
    SetControlRotation(FRotator(-12, 0, 0));
    PlayerCameraManager->ViewPitchMin = -65;
    PlayerCameraManager->ViewPitchMax = 30;
}
