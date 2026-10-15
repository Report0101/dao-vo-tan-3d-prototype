#include "GatePlayerController.h"
#include "Camera/PlayerCameraManager.h"
void AGatePlayerController::BeginPlay()
{
    Super::BeginPlay();
    SetInputMode(FInputModeGameOnly());
    bShowMouseCursor = false;
    SetControlRotation(FRotator(-12, 0, 0));
    PlayerCameraManager->ViewPitchMin = -65;
    PlayerCameraManager->ViewPitchMax = 30;
}
