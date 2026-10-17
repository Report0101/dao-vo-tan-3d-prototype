#include "PortraitComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "GameFramework/Character.h"
UPortraitComponent::UPortraitComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = .5f;
}
UTextureRenderTarget2D *UPortraitComponent::RequestPortrait()
{
    if (Target)
        return Target;
    auto *C = Cast<ACharacter>(GetOwner());
    if (!C)
        return nullptr;
    Target = NewObject<UTextureRenderTarget2D>(this);
    Target->ClearColor = FLinearColor(.016f, .028f, .038f, 1);
    Target->RenderTargetFormat = RTF_RGBA8;
    Target->InitAutoFormat(256, 256);
    KeyLight = NewObject<UPointLightComponent>(C);
    C->AddInstanceComponent(KeyLight);
    KeyLight->SetIntensityUnits(ELightUnits::Lumens);
    KeyLight->SetIntensity(100);
    KeyLight->SetAttenuationRadius(250);
    KeyLight->SetCastShadows(false);
    KeyLight->SetLightColor(FLinearColor(.8f, .9f, 1));
    KeyLight->LightingChannels.bChannel0 = false;
    KeyLight->LightingChannels.bChannel1 = true;
    KeyLight->RegisterComponent();
    Backdrop = NewObject<UStaticMeshComponent>(C);
    C->AddInstanceComponent(Backdrop);
    Backdrop->SetStaticMesh(
        LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane")));
    Backdrop->SetMaterial(0, LoadObject<UMaterialInterface>(
                                 nullptr, TEXT("/Game/DaoVoTan/Visual11/M_PortraitBackground")));
    Backdrop->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Backdrop->SetCastShadow(false);
    Backdrop->SetVisibleInSceneCaptureOnly(true);
    Backdrop->SetWorldScale3D(FVector(2));
    Backdrop->RegisterComponent();
    Capture = NewObject<USceneCaptureComponent2D>(C);
    C->AddInstanceComponent(Capture);
    Capture->TextureTarget = Target;
    Capture->bCaptureEveryFrame = false;
    Capture->bCaptureOnMovement = false;
    Capture->FOVAngle = 27;
    Capture->CaptureSource = SCS_FinalColorLDR;
    Capture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
    Capture->ShowOnlyActorComponents(C);
    Capture->ShowFlags.SetAtmosphere(false);
    Capture->ShowFlags.SetFog(false);
    Capture->ShowFlags.SetMotionBlur(false);
    Capture->RegisterComponent();
    Refresh();
    return Target;
}
void UPortraitComponent::Refresh()
{
    auto *C = Cast<ACharacter>(GetOwner());
    if (!C || !Capture)
        return;
    Capture->ShowOnlyComponents.Empty();
    Capture->ShowOnlyActorComponents(C);
    const FVector Focus = C->GetMesh()->GetSocketLocation(TEXT("head")) - FVector(0, 0, 9);
    const FVector Position =
        Focus + C->GetActorForwardVector() * 125 + C->GetActorRightVector() * 12 + FVector(0, 0, 6);
    KeyLight->SetWorldLocation(Position + FVector(0, 0, 25));
    Backdrop->SetWorldLocationAndRotation(
        Focus - C->GetActorForwardVector() * 60,
        FRotationMatrix::MakeFromZ(C->GetActorForwardVector()).Rotator());
    Capture->SetWorldLocationAndRotation(Position, (Focus - Position).Rotation());
    Capture->CaptureScene();
}
void UPortraitComponent::TickComponent(float Dt, ELevelTick Tick, FActorComponentTickFunction *Fn)
{
    Super::TickComponent(Dt, Tick, Fn);
    Age += Dt;
    if (Target && Age >= 1)
    {
        Age = 0;
        Refresh();
    }
}
