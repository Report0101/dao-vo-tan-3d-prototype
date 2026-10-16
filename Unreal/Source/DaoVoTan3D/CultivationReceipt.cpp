#include "CultivationReceipt.h"
#include "PresentationSnapshot.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Internationalization/Regex.h"
namespace
{
bool Text(const TSharedPtr<FJsonObject> &O, const TCHAR *K, FString &Out, bool Nullable = false)
{
    const auto *V = O->Values.Find(K);
    if (!V)
        return false;
    if (Nullable && (*V)->IsNull())
    {
        Out.Empty();
        return true;
    }
    return (*V)->Type == EJson::String && (*V)->TryGetString(Out) && !Out.IsEmpty() && Out.Len() <= 512;
}
bool Tick(const TSharedPtr<FJsonObject> &O, const TCHAR *K, int64 &Out)
{
    const auto *V = O->Values.Find(K);
    if (!V)
        return false;
    if ((*V)->IsNull())
    {
        Out = -1;
        return true;
    }
    double D;
    if ((*V)->Type != EJson::Number || !(*V)->TryGetNumber(D) || !FMath::IsFinite(D) || D < 0 ||
        D > 9007199254740991.0 || FMath::FloorToDouble(D) != D)
        return false;
    Out = int64(D);
    return true;
}
bool Pattern(const FString &S, const TCHAR *R)
{
    FRegexMatcher M(FRegexPattern(R), S);
    return M.FindNext() && M.GetMatchBeginning() == 0 && M.GetMatchEnding() == S.Len();
}
} // namespace
bool CultivationProtocol::ParseReceipt(const FString &Json, FCultivationReceipt &Out, FString &Error)
{
    FCultivationReceipt S;
    TSharedPtr<FJsonObject> O;
    auto Fail = [&]()
    {
        Error = TEXT("Unsupported/malformed cultivation receipt");
        return false;
    };
    if (FTCHARToUTF8(*Json).Length() > 65536 ||
        !FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), O) || !O || O->Values.Num() != 13)
        return Fail();
    const TCHAR *Fields[] = {TEXT("schemaVersion"),
                             TEXT("worldId"),
                             TEXT("requestId"),
                             TEXT("actorId"),
                             TEXT("status"),
                             TEXT("rejectionCode"),
                             TEXT("actionInstanceId"),
                             TEXT("acceptedEventId"),
                             TEXT("resultEventIds"),
                             TEXT("startTick"),
                             TEXT("dueTick"),
                             TEXT("resolvedTick"),
                             TEXT("cultivationSessionEventId")};
    for (auto *F : Fields)
        if (!O->HasField(F))
            return Fail();
    double Version = 0;
    if (O->Values[TEXT("schemaVersion")]->Type != EJson::Number ||
        !O->TryGetNumberField(TEXT("schemaVersion"), Version) || Version != 1)
        return Fail();
    if (!Text(O, TEXT("worldId"), S.WorldId) || !Pattern(S.WorldId, TEXT("[a-z][a-z0-9_-]{0,127}")) ||
        !Text(O, TEXT("requestId"), S.RequestId) || !Pattern(S.RequestId, TEXT("[A-Za-z0-9_-]{1,80}")) ||
        !Text(O, TEXT("actorId"), S.ActorId) || !Text(O, TEXT("status"), S.Status) ||
        !Text(O, TEXT("rejectionCode"), S.RejectionCode, true) ||
        !Text(O, TEXT("actionInstanceId"), S.ActionId, true) ||
        !Text(O, TEXT("acceptedEventId"), S.AcceptedEventId, true) ||
        !Text(O, TEXT("cultivationSessionEventId"), S.SessionEventId, true) ||
        !Tick(O, TEXT("startTick"), S.StartTick) || !Tick(O, TEXT("dueTick"), S.DueTick) ||
        !Tick(O, TEXT("resolvedTick"), S.ResolvedTick))
        return Fail();
    if (Pattern(S.RequestId.ToLower(), TEXT("(con|prn|aux|nul|com[0-9]+|lpt[0-9]+)")))
        return Fail();
    const TSet<FString> Codes = {TEXT("BAD_SCHEMA"),          TEXT("WRONG_WORLD"),
                                 TEXT("WRONG_ACTOR"),         TEXT("NOT_CONTROLLED"),
                                 TEXT("ACTOR_DEAD"),          TEXT("ACTION_BUSY"),
                                 TEXT("PREREQUISITE_FAILED"), TEXT("INCOMPATIBLE_STATE"),
                                 TEXT("REQUEST_CONFLICT"),    TEXT("SESSION_FULL")};
    const TArray<TSharedPtr<FJsonValue>> *Results = nullptr;
    if (!O->TryGetArrayField(TEXT("resultEventIds"), Results) || Results->Num() > 64)
        return Fail();
    TSet<FString> Seen;
    for (const auto &V : *Results)
    {
        FString Id;
        if (V->Type != EJson::String || !V->TryGetString(Id) ||
            !SnapshotV1::ValidateEntityId(Id, S.WorldId, TEXT("event")) || Seen.Contains(Id))
            return Fail();
        Seen.Add(Id);
        S.ResultEventIds.Add(Id);
    }
    if (S.Status == TEXT("rejected"))
    {
        if (!Codes.Contains(S.RejectionCode) || !S.ActionId.IsEmpty() || !S.AcceptedEventId.IsEmpty() ||
            S.StartTick != -1 || S.DueTick != -1 || S.ResolvedTick != -1 || !S.ResultEventIds.IsEmpty() ||
            !S.SessionEventId.IsEmpty())
            return Fail();
        TArray<FString> Parts;
        S.ActorId.ParseIntoArray(Parts, TEXT(":"), false);
        if (Parts.Num() != 3 || !SnapshotV1::ValidateEntityId(S.ActorId, Parts[0], TEXT("agent")))
            return Fail();
    }
    else
    {
        if (!SnapshotV1::ValidateEntityId(S.ActorId, S.WorldId, TEXT("agent")) ||
            !SnapshotV1::ValidateEntityId(S.ActionId, S.WorldId, TEXT("action")) ||
            !SnapshotV1::ValidateEntityId(S.AcceptedEventId, S.WorldId, TEXT("event")) || S.StartTick < 0 ||
            S.DueTick <= S.StartTick)
            return Fail();
        if (S.Status == TEXT("accepted"))
        {
            if (!S.RejectionCode.IsEmpty() || S.ResolvedTick != -1 || !S.ResultEventIds.IsEmpty() ||
                !S.SessionEventId.IsEmpty())
                return Fail();
        }
        else if (S.Status == TEXT("completed"))
        {
            if (!S.RejectionCode.IsEmpty() || S.ResolvedTick != S.DueTick ||
                !SnapshotV1::ValidateEntityId(S.SessionEventId, S.WorldId, TEXT("event")) ||
                !S.ResultEventIds.Contains(S.SessionEventId))
                return Fail();
        }
        else if (S.Status == TEXT("interrupted"))
        {
            if (!Codes.Contains(S.RejectionCode) || S.ResolvedTick < S.StartTick ||
                S.ResultEventIds.IsEmpty() || !S.SessionEventId.IsEmpty())
                return Fail();
        }
        else
            return Fail();
    }
    Out = MoveTemp(S);
    Error.Empty();
    return true;
}
