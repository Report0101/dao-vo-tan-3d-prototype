#include "GatePlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "CultivationCommandAdapter.h"
#include "EnhancedInputComponent.h"
#include "InputKeyEventArgs.h"
bool AGatePlayerController::InputKey(const FInputKeyEventArgs &Params)
{
    if (Params.Key == EKeys::E || Params.Key == EKeys::M || Params.Key == EKeys::LeftMouseButton)
        UE_LOG(LogTemp, Display,
               TEXT("INPUT_ACTION raw key=%s event=%d controller=%s pawn=%s adapter=%d"),
               *Params.Key.ToString(), int32(Params.Event), *GetName(), *GetNameSafe(GetPawn()),
               CultivationCommands != nullptr);
    return Super::InputKey(Params);
}
AGatePlayerController::AGatePlayerController()
{
    CultivationCommands =
        CreateDefaultSubobject<UCultivationCommandAdapter>(TEXT("CultivationCommands"));
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
