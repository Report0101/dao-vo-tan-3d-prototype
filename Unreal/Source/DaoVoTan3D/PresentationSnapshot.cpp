#include "PresentationSnapshot.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Internationalization/Regex.h"
namespace
{
bool Keys(const TSharedPtr<FJsonObject> &O, std::initializer_list<const TCHAR *> K)
{
    if (!O || O->Values.Num() != int32(K.size()))
        return false;
    for (auto *S : K)
        if (!O->HasField(S))
            return false;
    return true;
}
bool Str(const TSharedPtr<FJsonObject> &O, const TCHAR *K, FString &Out)
{
    const auto *V = O->Values.Find(K);
    return V && (*V)->Type == EJson::String && (*V)->TryGetString(Out) && !Out.IsEmpty() && Out.Len() <= 512;
}
bool Num(const TSharedPtr<FJsonObject> &O, const TCHAR *K, int64 &Out, int64 Min = 0)
{
    const auto *V = O->Values.Find(K);
    double D = 0;
    if (!V || (*V)->Type != EJson::Number || !(*V)->TryGetNumber(D) || !FMath::IsFinite(D) ||
        D < double(Min) || D > 9007199254740991.0 || FMath::FloorToDouble(D) != D)
        return false;
    Out = int64(D);
    return true;
}
bool Null(const TSharedPtr<FJsonObject> &O, const TCHAR *K)
{
    const auto *V = O->Values.Find(K);
    return V && (*V)->IsNull();
}
bool Match(const FString &S, const TCHAR *Pattern)
{
    FRegexMatcher M(FRegexPattern(Pattern), S);
    return M.FindNext() && M.GetMatchBeginning() == 0 && M.GetMatchEnding() == S.Len();
}
bool Id(const FString &S, const FString &World, const TCHAR *Kind)
{
    TArray<FString> Parts;
    S.ParseIntoArray(Parts, TEXT(":"), false);
    if (Parts.Num() != 3 || Parts[0] != World || Parts[1] != Kind || !Match(Parts[2], TEXT("[0-9]{8,}")))
        return false;
    double N = FCString::Atod(*Parts[2]);
    return N >= 1 && N <= 9007199254740991.0 && FMath::FloorToDouble(N) == N;
}
bool NullableId(const TSharedPtr<FJsonObject> &O, const TCHAR *K, FString &Out, const FString &World,
                const TCHAR *Kind)
{
    if (Null(O, K))
    {
        Out.Empty();
        return true;
    }
    return Str(O, K, Out) && Id(Out, World, Kind);
}
} // namespace
bool SnapshotV1::Parse(const FString &Json, FWorldPresentationSnapshot &Out, FString &Error)
{
    // Parse/validate into a temporary. Failed input never publishes partially parsed data.
    FWorldPresentationSnapshot S;
    TSharedPtr<FJsonObject> O;
    auto Fail = [&](const TCHAR *Reason)
    {
        Error = Reason;
        return false;
    };
    if (FTCHARToUTF8(*Json).Length() > 1024 * 1024 ||
        !FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), O))
        return Fail(TEXT("Malformed JSON or size limit"));
    if (!Keys(O, {TEXT("schemaVersion"), TEXT("worldId"), TEXT("rulesetVersion"), TEXT("simulationVersion"),
                  TEXT("streamId"), TEXT("sequence"), TEXT("worldTick"), TEXT("throughEventId"),
                  TEXT("controlledActorId"), TEXT("actors"), TEXT("actions")}))
        return Fail(TEXT("Snapshot field set mismatch"));
    int64 Version = 0;
    if (!Num(O, TEXT("schemaVersion"), Version) || Version != 1)
        return Fail(TEXT("Unsupported snapshot schema"));
    if (!Str(O, TEXT("worldId"), S.WorldId) || !Match(S.WorldId, TEXT("[a-z][a-z0-9_-]*")) ||
        !Str(O, TEXT("rulesetVersion"), S.RulesetVersion) ||
        !Str(O, TEXT("simulationVersion"), S.SimulationVersion) || !Str(O, TEXT("streamId"), S.StreamId) ||
        !Num(O, TEXT("sequence"), S.Sequence, 1) || !Num(O, TEXT("worldTick"), S.WorldTick))
        return Fail(TEXT("Invalid metadata"));
    if (S.RulesetVersion!=TEXT("rules-v2") || S.SimulationVersion!=TEXT("0.0.2"))
        return Fail(TEXT("Incompatible simulation versions: expected rules-v2 / 0.0.2"));
    if (!NullableId(O, TEXT("throughEventId"), S.ThroughEventId, S.WorldId, TEXT("event")) ||
        !NullableId(O, TEXT("controlledActorId"), S.ControlledActorId, S.WorldId, TEXT("agent")))
        return Fail(TEXT("Invalid envelope entity ID"));
    const TArray<TSharedPtr<FJsonValue>> *Actors = nullptr;
    const TArray<TSharedPtr<FJsonValue>> *Cues = nullptr;
    if (!O->TryGetArrayField(TEXT("actors"), Actors) || !O->TryGetArrayField(TEXT("actions"), Cues) ||
        Actors->Num() > 10 || Cues->Num() > 256)
        return Fail(TEXT("Array limits"));
    TSet<FString> ActorIds;
    FString Previous;
    for (auto &V : *Actors)
    {
        if (V->Type != EJson::Object)
            return Fail(TEXT("Invalid actor object"));
        auto A = V->AsObject();
        FSnapshotActorState State;
        if (!Keys(A,
                  {TEXT("actorId"), TEXT("displayName"), TEXT("location"), TEXT("behaviorStatus"),
                   TEXT("lifeStatus"), TEXT("cultivation"), TEXT("appearanceSeed"), TEXT("currentAction")}))
            return Fail(TEXT("Actor field set mismatch"));
        if (!Str(A, TEXT("actorId"), State.ActorId) || !Id(State.ActorId, S.WorldId, TEXT("agent")) ||
            ActorIds.Contains(State.ActorId) ||
            (!Previous.IsEmpty() && Previous.Compare(State.ActorId, ESearchCase::CaseSensitive) >= 0) ||
            !Str(A, TEXT("displayName"), State.DisplayName))
            return Fail(TEXT("Invalid/unsorted actor ID"));
        Previous = State.ActorId;
        ActorIds.Add(State.ActorId);
        const TSharedPtr<FJsonObject> *Location = nullptr;
        if (!A->TryGetObjectField(TEXT("location"), Location) || !Keys(*Location, {TEXT("regionId")}) ||
            !Str(*Location, TEXT("regionId"), State.RegionId) ||
            !Id(State.RegionId, S.WorldId, TEXT("region")))
            return Fail(TEXT("Invalid region location"));
        if (!Str(A, TEXT("behaviorStatus"), State.BehaviorStatus) ||
            (State.BehaviorStatus != TEXT("active") && State.BehaviorStatus != TEXT("inactive")) ||
            !Str(A, TEXT("lifeStatus"), State.LifeStatus) ||
            (State.LifeStatus != TEXT("alive") && State.LifeStatus != TEXT("dead") &&
             State.LifeStatus != TEXT("unknown")))
            return Fail(TEXT("Invalid status enum"));
        if (!Null(A, TEXT("currentAction")) || !Str(A, TEXT("appearanceSeed"), State.AppearanceSeed) ||
            !Match(State.AppearanceSeed, TEXT("[a-f0-9]{64}")))
            return Fail(TEXT("Invalid appearance/action"));
        if (!Null(A, TEXT("cultivation")))
        {
            const TSharedPtr<FJsonObject> *C = nullptr;
            if (!A->TryGetObjectField(TEXT("cultivation"), C) ||
                !Keys(*C, {TEXT("hasMethod"), TEXT("completedSessions")}))
                return Fail(TEXT("Invalid cultivation"));
            const auto *B = (*C)->Values.Find(TEXT("hasMethod"));
            if (!B || (*B)->Type != EJson::Boolean || !(*B)->TryGetBool(State.bHasMethod) ||
                !Num(*C, TEXT("completedSessions"), State.CompletedSessions))
                return Fail(TEXT("Invalid cultivation values"));
            State.bHasCultivation = true;
        }
        S.Actors.Add(State);
    }
    if (!S.ControlledActorId.IsEmpty() && !ActorIds.Contains(S.ControlledActorId))
        return Fail(TEXT("Controlled actor absent"));
    TSet<FString> EventIds;
    int64 PreviousTick = 0;
    for (auto &V : *Cues)
    {
        if (V->Type != EJson::Object)
            return Fail(TEXT("Invalid action object"));
        auto A = V->AsObject();
        FSnapshotCue C;
        FString Type, Phase;
        if (!Keys(A, {TEXT("actorId"), TEXT("actionInstanceId"), TEXT("actionType"), TEXT("phase"),
                      TEXT("occurredAtTick")}))
            return Fail(TEXT("Action field set mismatch"));
        if (!Str(A, TEXT("actorId"), C.ActorId) || !ActorIds.Contains(C.ActorId) ||
            !Str(A, TEXT("actionInstanceId"), C.EventId) || !Id(C.EventId, S.WorldId, TEXT("event")) ||
            EventIds.Contains(C.EventId) || !Str(A, TEXT("actionType"), Type) ||
            Type != TEXT("cultivation_session_completed") || !Str(A, TEXT("phase"), Phase) ||
            Phase != TEXT("completed") || !Num(A, TEXT("occurredAtTick"), C.Tick) || C.Tick < PreviousTick ||
            C.Tick > S.WorldTick || S.ThroughEventId.IsEmpty())
            return Fail(TEXT("Invalid action cue"));
        EventIds.Add(C.EventId);
        PreviousTick = C.Tick;
        S.Cues.Add(C);
    }
    Out = MoveTemp(S);
    Error.Empty();
    return true;
}

bool SnapshotV1::ValidateEntityId(const FString &Value, const FString &World, const TCHAR *Kind)
{
    return Match(World, TEXT("[a-z][a-z0-9_-]*")) && Id(Value, World, Kind);
}
