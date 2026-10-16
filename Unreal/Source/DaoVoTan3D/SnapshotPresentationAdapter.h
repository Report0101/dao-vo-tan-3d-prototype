#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PresentationSnapshot.h"
#include "SnapshotPresentationAdapter.generated.h"
UCLASS(ClassGroup = Presentation, meta = (BlueprintSpawnableComponent))
class DAOVOTAN3D_API USnapshotPresentationAdapter : public UActorComponent
{
    GENERATED_BODY()
  public:
    USnapshotPresentationAdapter();
    void SetSlots(class AGateCharacter *Player, const TArray<AGateCharacter *> &NPCs);
    bool ApplyJson(const FString &Json, bool bFullResync, FString &Error);
    UFUNCTION(BlueprintCallable) bool LoadSnapshotFile(const FString &Path, bool bFullResync);
    virtual void TickComponent(float Dt, ELevelTick Tick, FActorComponentTickFunction *Fn) override;
    const FSnapshotActorState *StateFor(const AGateCharacter *Actor) const;
    AGateCharacter *ActorFor(const FString &Id) const;
    void SetCommandPreviewActor(const FString &Id, bool Owned)
    {
        if (Owned)
            CommandPreviewActors.Add(Id);
        else
            CommandPreviewActors.Remove(Id);
    }
    int32 BindingCount() const
    {
        return Bindings.Num();
    }
    int32 UniqueCueCount = 0, PlayedPreviewCount = 0, DuplicateCueCount = 0, RejectedCount = 0;
    double LastParseMs = 0, LastApplyMs = 0;
    int64 LastSequence() const
    {
        return Current.Sequence;
    }
    const FWorldPresentationSnapshot &Snapshot() const
    {
        return Current;
    }

  private:
    UPROPERTY() TArray<TObjectPtr<AGateCharacter>> Slots;
    UPROPERTY() TMap<FString, TObjectPtr<AGateCharacter>> Bindings;
    FWorldPresentationSnapshot Current;
    TMap<FString, FString> SeenCues;
    TSet<FString> RetiredStreams;
    TSet<FString> CommandPreviewActors;
    TArray<FSnapshotCue> Pending;
    TMap<FString, float> Playing;
    FString FilePath, LastFileContent;
    float PollAge = 0;
    void ApplyAppearance(AGateCharacter *Actor, const FString &Seed);
};
