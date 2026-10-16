#pragma once
#include "CoreMinimal.h"
// Owned transport DTOs; no Actor, UObject, asset path or authoritative mutation handle.
struct FSnapshotActorState
{
    FString ActorId, DisplayName, RegionId, BehaviorStatus, LifeStatus, AppearanceSeed;
    bool bHasCultivation = false, bHasMethod = false;
    int64 CompletedSessions = 0;
};
struct FSnapshotCue
{
    FString ActorId, EventId;
    int64 Tick = 0;
};
struct FWorldPresentationSnapshot
{
    FString WorldId, RulesetVersion, SimulationVersion, StreamId, ThroughEventId, ControlledActorId;
    int64 Sequence = 0, WorldTick = 0;
    TArray<FSnapshotActorState> Actors;
    TArray<FSnapshotCue> Cues;
};
namespace SnapshotV1
{
bool ValidateEntityId(const FString &Id, const FString &World, const TCHAR *Kind);
DAOVOTAN3D_API bool Parse(const FString &Json, FWorldPresentationSnapshot &Out, FString &Error);
} // namespace SnapshotV1
