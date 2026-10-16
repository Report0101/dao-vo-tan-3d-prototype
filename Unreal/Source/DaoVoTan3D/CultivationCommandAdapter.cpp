#include "CultivationCommandAdapter.h"
#include "SnapshotPresentationAdapter.h"
#include "GateCharacter.h"
#include "PresentationActions.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
UCultivationCommandAdapter::UCultivationCommandAdapter()
{
    PrimaryComponentTick.bCanEverTick = true;
}
void UCultivationCommandAdapter::BeginPlay()
{
    Super::BeginPlay();
    FParse::Value(FCommandLine::Get(), TEXT("CultivationBridgeDir="), BridgeDir);
    FParse::Value(FCommandLine::Get(), TEXT("CultivationDays="), SelectedDays);
    SelectedDays = FMath::Clamp(SelectedDays, 1, 30);
    bTransportEnabled =
        !BridgeDir.IsEmpty() && !FParse::Param(FCommandLine::Get(), TEXT("CultivationGate2Test"));
}
USnapshotPresentationAdapter *UCultivationCommandAdapter::SnapshotAdapter() const
{
    auto *Mode = GetWorld()->GetAuthGameMode();
    return Mode ? Mode->FindComponentByClass<USnapshotPresentationAdapter>() : nullptr;
}
void UCultivationCommandAdapter::BindInputs(UEnhancedInputComponent *Input)
{
    auto *PC = Cast<APlayerController>(GetOwner());
    if (!PC || !PC->GetLocalPlayer() || !Input ||
        FParse::Param(FCommandLine::Get(), TEXT("GateDeveloperPreview")))
        return;
    InputContext = NewObject<UInputMappingContext>(this);
    auto Bind = [&](FKey Key, auto Method)
    {
        auto *Action = NewObject<UInputAction>(this);
        InputActions.Add(Action);
        InputContext->MapKey(Action, Key);
        Input->BindAction(Action, ETriggerEvent::Started, this, Method);
    };
    Bind(EKeys::E, &UCultivationCommandAdapter::RequestCultivate);
    Bind(EKeys::M, &UCultivationCommandAdapter::RequestCultivate);
    Bind(EKeys::Equals, &UCultivationCommandAdapter::MoreDays);
    Bind(EKeys::Hyphen, &UCultivationCommandAdapter::FewerDays);
    if (auto *Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        Subsystem->AddMappingContext(InputContext, 10);
}
bool UCultivationCommandAdapter::WriteRequest()
{
    if (!IsConnected())
        return false;
    const FString Dir = FPaths::Combine(BridgeDir, TEXT("inbox"));
    IFileManager::Get().MakeDirectory(*Dir, true);
    const FString File = FPaths::Combine(Dir, LastRequestId + TEXT(".json")),
                  Tmp = File + TEXT(".") + FGuid::NewGuid().ToString(EGuidFormats::Digits) + TEXT(".tmp");
    return FFileHelper::SaveStringToFile(LastRequestJson, *Tmp,
                                         FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM) &&
           IFileManager::Get().Move(*File, *Tmp, true, true, false, true);
}
void UCultivationCommandAdapter::RequestCultivate()
{
    auto *A = SnapshotAdapter();
    if (!A || A->Snapshot().ControlledActorId.IsEmpty())
    {
        Feedback = TEXT("No controlled simulation actor bound");
        return;
    }
    SendCultivateForActor(A->Snapshot().ControlledActorId, SelectedDays);
}
bool UCultivationCommandAdapter::SendCultivateForActor(const FString &ActorId, int32 Days)
{
    auto *A = SnapshotAdapter();
    if (!IsConnected() || !A || A->Snapshot().WorldId.IsEmpty())
    {
        Feedback = TEXT("Cultivation simulator not connected");
        return false;
    }
    if (Receipt.Status == TEXT("accepted") || Receipt.Status == TEXT("sent"))
    {
        Feedback = TEXT("Awaiting authoritative result");
        return false;
    }
    const double Begin = FPlatformTime::Seconds();
    LastRequestId = FGuid::NewGuid().ToString(EGuidFormats::Digits);
    ExpectedActor = ActorId;
    TSharedRef<FJsonObject> O = MakeShared<FJsonObject>();
    O->SetNumberField(TEXT("schemaVersion"), 1);
    O->SetStringField(TEXT("worldId"), A->Snapshot().WorldId);
    O->SetStringField(TEXT("requestId"), LastRequestId);
    O->SetStringField(TEXT("actorId"), ActorId);
    TSharedRef<FJsonObject> Command = MakeShared<FJsonObject>();
    Command->SetStringField(TEXT("type"), TEXT("cultivate"));
    Command->SetNumberField(TEXT("days"), Days);
    O->SetObjectField(TEXT("command"), Command);
    LastRequestJson.Empty();
    FJsonSerializer::Serialize(O, TJsonWriterFactory<>::Create(&LastRequestJson));
    LastSerializeMs = (FPlatformTime::Seconds() - Begin) * 1000;
    if (!WriteRequest())
    {
        Feedback = TEXT("Could not write cultivation request");
        return false;
    }
    Receipt = FCultivationReceipt();
    Receipt.Status = TEXT("sent");
    LastReceiptJson.Empty();
    Feedback = TEXT("Cultivate request sent / awaiting simulation");
    A->SetCommandPreviewActor(ActorId, true);
    UE_LOG(LogTemp, Display, TEXT("GATE2 sent request=%s actor=%s days=%d"), *LastRequestId, *ActorId, Days);
    return true;
}
bool UCultivationCommandAdapter::ResendLastRequest()
{
    return !LastRequestId.IsEmpty() && WriteRequest();
}
void UCultivationCommandAdapter::StartPreview()
{
    if (PreviewedRequests.Contains(Receipt.RequestId))
        return;
    auto *A = SnapshotAdapter();
    auto *Actor = A ? A->ActorFor(Receipt.ActorId) : nullptr;
    if (!Actor)
        return;
    const auto *State = A->StateFor(Actor);
    if (State && State->LifeStatus == TEXT("dead"))
        return;
    PreviewedRequests.Add(Receipt.RequestId);
    VisualActor = Actor;
    VisualAge = 0;
    Actor->Actions->ResetToLocomotionPresentation();
    Actor->Actions->StartMeditationPresentation();
    ++PreviewStartCount;
    UE_LOG(LogTemp, Display, TEXT("GATE2 cosmetic preview request=%s action=%s"), *Receipt.RequestId,
           *Receipt.ActionId);
}
bool UCultivationCommandAdapter::ConsumeReceipt(const FString &Json, FString &Error)
{
    const double Begin = FPlatformTime::Seconds();
    FCultivationReceipt Next;
    if (!CultivationProtocol::ParseReceipt(Json, Next, Error))
    {
        ++RejectedReceiptCount;
        return false;
    }
    LastParseMs = (FPlatformTime::Seconds() - Begin) * 1000;
    const double Update = FPlatformTime::Seconds();
    auto *A = SnapshotAdapter();
    if (!A || Next.RequestId != LastRequestId || Next.ActorId != ExpectedActor ||
        Next.WorldId != A->Snapshot().WorldId)
    {
        Error = TEXT("Receipt identity mismatch");
        ++RejectedReceiptCount;
        return false;
    }
    if (ResolvedRequests.Contains(Next.RequestId))
        return true;
    if (Receipt.Status == TEXT("accepted") &&
        (Next.ActionId != Receipt.ActionId || Next.AcceptedEventId != Receipt.AcceptedEventId ||
         Next.StartTick != Receipt.StartTick || Next.DueTick != Receipt.DueTick))
    {
        Error = TEXT("Receipt changed accepted action identity");
        ++RejectedReceiptCount;
        return false;
    }
    Receipt = MoveTemp(Next);
    bTerminal = Receipt.Status != TEXT("accepted");
    if (Receipt.Status == TEXT("accepted") || Receipt.Status == TEXT("completed"))
        StartPreview();
    if (Receipt.Status == TEXT("accepted"))
        Feedback = FString::Printf(TEXT("Cultivation accepted / strategic tick %lld -> %lld"),
                                   Receipt.StartTick, Receipt.DueTick);
    else
    {
        ResolvedRequests.Add(Receipt.RequestId);
        ++LogicalResultCount;
        if (Receipt.Status == TEXT("completed"))
            Feedback = FString::Printf(TEXT("Cultivation committed at tick %lld / %s"), Receipt.ResolvedTick,
                                       *Receipt.SessionEventId);
        else
        {
            Feedback = Receipt.Status == TEXT("rejected") ? TEXT("Cultivation rejected: ")
                                                          : TEXT("Cultivation interrupted: ");
            if (Receipt.RejectionCode == TEXT("ACTION_BUSY"))
                Feedback += TEXT("actor already has an exclusive action");
            else if (Receipt.RejectionCode == TEXT("ACTOR_DEAD"))
                Feedback += TEXT("actor is not alive");
            else if (Receipt.RejectionCode == TEXT("PREREQUISITE_FAILED"))
                Feedback += TEXT("cultivation prerequisites are missing");
            else if (Receipt.RejectionCode == TEXT("NOT_CONTROLLED") ||
                     Receipt.RejectionCode == TEXT("WRONG_ACTOR"))
                Feedback += TEXT("actor is not the controlled character");
            else
                Feedback += Receipt.RejectionCode;
            if (!VisualActor)
                A->SetCommandPreviewActor(ExpectedActor, false);
        }
    }
    LastUpdateMs = (FPlatformTime::Seconds() - Update) * 1000;
    UE_LOG(LogTemp, Display, TEXT("GATE2 receipt request=%s status=%s action=%s result_count=%d"),
           *Receipt.RequestId, *Receipt.Status, *Receipt.ActionId, Receipt.ResultEventIds.Num());
    return true;
}
void UCultivationCommandAdapter::TickComponent(float Dt, ELevelTick Tick, FActorComponentTickFunction *Fn)
{
    Super::TickComponent(Dt, Tick, Fn);
    if (VisualActor)
    {
        VisualAge += Dt;
        auto *A = SnapshotAdapter();
        const auto *State = A ? A->StateFor(VisualActor) : nullptr;
        const bool Dead = State && State->LifeStatus == TEXT("dead");
        if (Dead || (bTerminal && VisualAge > 2.8f))
            VisualActor->Actions->StopMeditationPresentation();
        if (Dead || (bTerminal && VisualAge > 4.2f))
        {
            VisualActor->Actions->ResetToLocomotionPresentation();
            if (Dead)
                VisualActor->GetCharacterMovement()->DisableMovement();
            if (A)
                A->SetCommandPreviewActor(Receipt.ActorId, false);
            VisualActor = nullptr;
        }
    }
    if (!IsConnected())
        return;
    PollAge += Dt;
    if (PollAge < .1f)
        return;
    PollAge = 0;
    FString Text, Error;
    if (!LastRequestId.IsEmpty())
    {
        const FString Path = FPaths::Combine(BridgeDir, TEXT("outbox"), LastRequestId + TEXT(".json"));
        if (IFileManager::Get().FileSize(*Path) <= 65536 && FFileHelper::LoadFileToString(Text, *Path) &&
            Text != LastReceiptJson)
        {
            if (ConsumeReceipt(Text, Error))
                LastReceiptJson = Text;
        }
    }
    const FString Path = FPaths::Combine(BridgeDir, TEXT("snapshot.json"));
    Text.Empty();
    if (IFileManager::Get().FileSize(*Path) <= 1024 * 1024 && FFileHelper::LoadFileToString(Text, *Path) &&
        Text != LastSnapshotJson)
    {
        FWorldPresentationSnapshot Next;
        auto *A = SnapshotAdapter();
        if (A && SnapshotV1::Parse(Text, Next, Error) &&
            A->ApplyJson(Text, A->Snapshot().StreamId != Next.StreamId, Error))
            LastSnapshotJson = Text;
    }
}
