#include "CultivatorLocomotion.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimNode_SequencePlayer.h"
#include "AnimNodes/AnimNode_TwoWayBlend.h"
#include "Animation/AnimNodeSpaceConversions.h"
#include "BoneControllers/AnimNode_ModifyBone.h"
#include "BoneControllers/AnimNode_CopyBone.h"
#include "BoneControllers/AnimNode_TwoBoneIK.h"
#include "BoneControllers/AnimNode_FootPlacement.h"
#include "Chooser.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "PoseSearch/AnimNode_MotionMatching.h"
#include "PoseSearch/AnimNode_PoseSearchHistoryCollector.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "PoseSearch/PoseSearchIndex.h"
#include "PoseSearch/PoseSearchTrajectoryLibrary.h"

namespace
{
// The sample remaps its 0..1 contact curves into the speed convention expected
// by FootPlacement. Keep that small conversion without importing CurveExpression.
struct FContactSpeedNode : FAnimNode_Base
{
    FPoseLink Source;
    virtual void Initialize_AnyThread(const FAnimationInitializeContext &C) override
    {
        Source.Initialize(C);
    }
    virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext &C) override
    {
        Source.CacheBones(C);
    }
    virtual void Update_AnyThread(const FAnimationUpdateContext &C) override
    {
        Source.Update(C);
    }
    virtual void Evaluate_AnyThread(FPoseContext &Output) override
    {
        Source.Evaluate(Output);
        for (FName Name : {FName(TEXT("contact_l")), FName(TEXT("contact_r"))})
            Output.Curve.Set(Name, (1.f - FMath::Clamp(Output.Curve.Get(Name), 0.f, 1.f)) * 100.f);
    }
};
struct FCultivatorProxy : FAnimInstanceProxy
{
    FAnimNode_MotionMatching Matching;
    FAnimNode_SequencePlayer_Standalone Action;
    FAnimNode_TwoWayBlend ActionBlend;
    FContactSpeedNode Contacts;
    FAnimNode_PoseSearchHistoryCollector History;
    FAnimNode_ConvertLocalToComponentSpace ToComponent;
    FAnimNode_ModifyBone Lean;
    FAnimNode_CopyBone CopyL, CopyR;
    FAnimNode_FootPlacement Feet;
    FAnimNode_TwoBoneIK LegL, LegR;
    FAnimNode_ConvertComponentToLocalSpace ToLocal;
    explicit FCultivatorProxy(UAnimInstance *Instance) : FAnimInstanceProxy(Instance)
    {
        // Configure reflected node settings through the same text serializer
        // used for authored graph defaults; no engine source changes.
        FAnimNode_MotionMatching::StaticStruct()->ImportText(
            TEXT("(BlendTime=0.18,PlayRate=(Min=0.85,Max=1.5),SearchThrottleTime=0.033)"),
            &Matching, nullptr, PPF_None, GWarn, TEXT("CultivatorMotionMatching"));
        ActionBlend.A.SetLinkNode(&Matching);
        ActionBlend.B.SetLinkNode(&Action);
        Contacts.Source.SetLinkNode(&ActionBlend);
        ToComponent.LocalPose.SetLinkNode(&Contacts);
        Lean.ComponentPose.SetLinkNode(&ToComponent);
        Lean.BoneToModify.BoneName = TEXT("spine_02");
        Lean.RotationMode = BMM_Additive;
        Lean.RotationSpace = BCS_ComponentSpace;
        Lean.Alpha = 1;
        auto ConfigureCopy = [](FAnimNode_CopyBone &Node, const TCHAR *Side)
        {
            Node.SourceBone.BoneName = FName(FString(TEXT("foot_")) + Side);
            Node.TargetBone.BoneName = FName(FString(TEXT("ik_foot_")) + Side);
            Node.bCopyTranslation = Node.bCopyRotation = true;
            Node.ControlSpace = BCS_ComponentSpace;
            Node.Alpha = 1;
        };
        ConfigureCopy(CopyL, TEXT("l"));
        ConfigureCopy(CopyR, TEXT("r"));
        CopyL.ComponentPose.SetLinkNode(&Lean);
        CopyR.ComponentPose.SetLinkNode(&CopyL);
        Feet.ComponentPose.SetLinkNode(&CopyR);
        Feet.PelvisBone.BoneName = TEXT("pelvis");
        Feet.IKFootRootBone.BoneName = TEXT("ik_foot_root");
        Feet.Alpha = 1;
        for (const TCHAR *Side : {TEXT("l"), TEXT("r")})
        {
            FFootPlacemenLegDefinition Leg;
            Leg.FKFootBone.BoneName = FName(FString(TEXT("foot_")) + Side);
            Leg.IKFootBone.BoneName = FName(FString(TEXT("ik_foot_")) + Side);
            Leg.BallBone.BoneName = FName(FString(TEXT("ball_")) + Side);
            Leg.SpeedCurveName = FName(FString(TEXT("contact_")) + Side);
            Feet.LegDefinitions.Add(Leg);
        }
        auto ConfigureLeg = [](FAnimNode_TwoBoneIK &Node, const TCHAR *Side, float Sign)
        {
            Node.IKBone.BoneName = FName(FString(TEXT("foot_")) + Side);
            Node.EffectorTarget.BoneReference.BoneName = FName(FString(TEXT("ik_foot_")) + Side);
            Node.EffectorLocationSpace = BCS_BoneSpace;
            Node.JointTargetLocationSpace = BCS_ComponentSpace;
            Node.JointTargetLocation = FVector(Sign * 20, 80, 40);
            Node.bTakeRotationFromEffectorSpace = true;
            Node.Alpha = 1;
        };
        ConfigureLeg(LegL, TEXT("l"), -1);
        ConfigureLeg(LegR, TEXT("r"), 1);
        LegL.ComponentPose.SetLinkNode(&Feet);
        LegR.ComponentPose.SetLinkNode(&LegL);
        ToLocal.ComponentPose.SetLinkNode(&LegR);
        History.Source.SetLinkNode(&ToLocal);
        History.PoseCount = 12;
        History.SamplingInterval = .04f;
        History.bGenerateTrajectory = false;
        History.TrajectoryPredictionCount = 12;
        History.PredictionSamplingInterval = .1f;
    }
    virtual FAnimNode_Base *GetCustomRootNode() override
    {
        return &History;
    }
    virtual void GetCustomNodes(TArray<FAnimNode_Base *> &Nodes) override
    {
        Nodes = {&Matching, &Action, &ActionBlend, &Contacts, &ToComponent, &Lean,   &CopyL,
                 &CopyR,    &Feet,   &LegL,        &LegR,     &ToLocal,     &History};
    }
    virtual void PreUpdate(UAnimInstance *Instance, float Dt) override
    {
        FAnimInstanceProxy::PreUpdate(Instance, Dt);
        auto *State = CastChecked<UCultivatorLocomotion>(Instance);
        History.TransformTrajectory = State->Trajectory;
        if (Action.GetSequence() != State->ActionClip)
        {
            Action.SetSequence(State->ActionClip);
            Action.SetAccumulatedTime(0);
        }
        Action.SetLoopAnimation(State->bActionLoop);
        ActionBlend.Alpha = FMath::FInterpConstantTo(
            ActionBlend.Alpha, State->bActionActive && State->ActionClip ? 1.f : 0.f, Dt, 6.f);
        if (State->Database)
            Matching.SetDatabaseToSearch(
                State->Database,
                EPoseSearchInterruptMode::InterruptOnDatabaseChangeAndInvalidateContinuingPose);
        const FVector A = State->LocalAcceleration;
        Lean.Rotation = FRotator(FMath::Clamp(A.X * -.003f, -7.f, 7.f), 0,
                                 FMath::Clamp(A.Y * .004f, -9.f, 9.f));
        Feet.Alpha = LegL.Alpha = LegR.Alpha = State->bAirborne || State->bActionActive ? 0.f : 1.f;
        Lean.Alpha = State->bActionActive ? 0.f : 1.f;
    }
    virtual void PostUpdate(UAnimInstance *Instance) const override
    {
        FAnimInstanceProxy::PostUpdate(Instance);
        auto *State = CastChecked<UCultivatorLocomotion>(Instance);
        const FString Name =
            GetNameSafe(Matching.GetMotionMatchingState().SearchResult.SelectedAnim);
        if (Name != State->SelectedAnimation)
            ++State->PoseSelections;
        State->SelectedAnimation = Name;
    }
};
} // namespace
UCultivatorLocomotion::UCultivatorLocomotion()
{
    RootMotionMode = ERootMotionMode::IgnoreRootMotion;
}
FAnimInstanceProxy *UCultivatorLocomotion::CreateAnimInstanceProxy()
{
    return new FCultivatorProxy(this);
}
void UCultivatorLocomotion::DestroyAnimInstanceProxy(FAnimInstanceProxy *Proxy)
{
    delete Proxy;
}
void UCultivatorLocomotion::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    Chooser = LoadObject<UChooserTable>(nullptr, TEXT("/Game/DaoVoTan/Locomotion/CHT_Locomotion"));
}
void UCultivatorLocomotion::NativeUpdateAnimation(float Dt)
{
    Super::NativeUpdateAnimation(Dt);
    auto *C = Cast<ACharacter>(TryGetPawnOwner());
    if (!C)
        return;
    auto *Move = C->GetCharacterMovement();
    bAirborne = Move->IsFalling();
    if (bWasAirborne && !bAirborne)
        LandingAge = .45f;
    else
        LandingAge = FMath::Max(0.f, LandingAge - Dt);
    bWasAirborne = bAirborne;
    bLanding = !bAirborne && LandingAge > 0;
    Speed = C->GetVelocity().Size2D();
    bRunning = Move->MaxWalkSpeed > 400;
    bMoving = Speed > 10 || !Move->GetCurrentAcceleration().IsNearlyZero();
    const float Yaw = C->GetActorRotation().Yaw;
    if (bMoving)
        TurnHoldRemaining = 0;
    else if (FMath::Abs(FMath::FindDeltaAngleDegrees(PreviousYaw, Yaw)) > .5f)
        TurnHoldRemaining = .4f;
    else
        TurnHoldRemaining = FMath::Max(0.f, TurnHoldRemaining - Dt);
    bTurning = TurnHoldRemaining > 0;
    PreviousYaw = Yaw;
    LocalAcceleration =
        C->GetActorTransform().InverseTransformVectorNoScale(Move->GetCurrentAcceleration());
    FPoseSearchTrajectoryData Prediction;
    Prediction.RotateTowardsMovementSpeed = 8;
    UPoseSearchTrajectoryLibrary::PoseSearchGenerateTransformTrajectory(
        this, Prediction, Dt, RawTrajectory, DesiredYawLastUpdate, Trajectory, .04f, 10, .1f, 12);
    // Correct the presentation query for gravity/floors without moving the pawn.
    FCollisionQueryParams Query(SCENE_QUERY_STAT(CultivatorTrajectory), false, C);
    for (auto &Sample : Trajectory.Samples)
    {
        if (Sample.TimeInSeconds <= 0)
            continue;
        if (bAirborne)
            Sample.Position.Z += .5f * Move->GetGravityZ() * FMath::Square(Sample.TimeInSeconds);
        FHitResult Hit;
        if (C->GetWorld()->LineTraceSingleByChannel(Hit, Sample.Position + FVector(0, 0, 150),
                                                    Sample.Position - FVector(0, 0, 500),
                                                    ECC_Visibility, Query))
            Sample.Position.Z =
                bAirborne ? FMath::Max(Sample.Position.Z, Hit.ImpactPoint.Z) : Hit.ImpactPoint.Z;
    }
    FChooserEvaluationContext Context;
    Context.AddObjectParam(this);
    UObject *Selected = nullptr;
    UChooserTable::EvaluateChooser(Context, Chooser,
                                   FObjectChooserBase::FObjectChooserIteratorCallback::CreateLambda(
                                       [&](UObject *Result)
                                       {
                                           Selected = Result;
                                           return FObjectChooserBase::EIteratorStatus::Stop;
                                       }));
    Database = Cast<UPoseSearchDatabase>(Selected);
    SelectedDatabase = GetNameSafe(Database);
    if (Database)
        SeenDatabases.Add(SelectedDatabase);
}
