#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CultivationReceipt.h"
#include "CultivationCommandAdapter.generated.h"
UCLASS(ClassGroup = Presentation)
class DAOVOTAN3D_API UCultivationCommandAdapter : public UActorComponent
{
    GENERATED_BODY()
  public:
    UCultivationCommandAdapter();
    virtual void BeginPlay() override;
    virtual void TickComponent(float Dt, ELevelTick Tick, FActorComponentTickFunction *Fn) override;
    void BindInputs(class UEnhancedInputComponent *Input);
    class UInputAction *CultivateInputAction() const
    {
        return InputActions.Num() ? InputActions[0].Get() : nullptr;
    }
    class UInputMappingContext *CommandInputContext() const
    {
        return InputContext;
    }
    UFUNCTION(BlueprintCallable) void RequestCultivate();
    bool SendCultivateForActor(const FString &ActorId, int32 Days);
    bool ResendLastRequest();
    bool ConsumeReceipt(const FString &Json, FString &Error);
    void SetTransportEnabled(bool Enabled)
    {
        bTransportEnabled = Enabled;
    }
    int32 SelectedDays = 1, PreviewStartCount = 0, LogicalResultCount = 0, RejectedReceiptCount = 0;
    double LastParseMs = 0, LastUpdateMs = 0, LastSerializeMs = 0;
    FString Feedback = TEXT("Cultivation simulator not connected"), LastRequestId, LastRequestJson,
            ExpectedActor;
    FCultivationReceipt Receipt;
    int32 InputAttempts = 0;
    FString InputStatus;
    bool IsConnected() const
    {
        return bTransportEnabled && !BridgeDir.IsEmpty();
    }

  private:
    FString BridgeDir, LastSnapshotJson, LastReceiptJson;
    bool bTransportEnabled = false, bTerminal = false;
    float PollAge = 0, VisualAge = 0;
    TSet<FString> PreviewedRequests, ResolvedRequests;
    UPROPERTY() TObjectPtr<class AGateCharacter> VisualActor;
    UPROPERTY() TObjectPtr<class UInputMappingContext> InputContext;
    UPROPERTY() TArray<TObjectPtr<class UInputAction>> InputActions;
    class USnapshotPresentationAdapter *SnapshotAdapter() const;
    void StartPreview();
    void CultivateStarted(const struct FInputActionInstance &Instance);
    void CultivateCompleted(const struct FInputActionInstance &Instance);
    void MoreDays()
    {
        SelectedDays = FMath::Min(30, SelectedDays + 1);
    }
    void FewerDays()
    {
        SelectedDays = FMath::Max(1, SelectedDays - 1);
    }
    bool WriteRequest();
};
