#pragma once
#include "CoreMinimal.h"
struct FCultivationReceipt
{
    FString WorldId, RequestId, ActorId, Status, RejectionCode, ActionId, AcceptedEventId, SessionEventId;
    TArray<FString> ResultEventIds;
    int64 StartTick = -1, DueTick = -1, ResolvedTick = -1;
};
namespace CultivationProtocol
{
bool ParseReceipt(const FString &Json, FCultivationReceipt &Out, FString &Error);
}
