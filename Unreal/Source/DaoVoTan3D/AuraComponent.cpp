#include "AuraComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
UAuraComponent::UAuraComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}
void UAuraComponent::BeginPlay()
{
    Super::BeginPlay();
    Motes = NewObject<UInstancedStaticMeshComponent>(GetOwner());
    GetOwner()->AddInstanceComponent(Motes);
    Motes->SetupAttachment(this);
    Motes->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")));
    Motes->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Motes->SetCastShadow(false);
    Motes->RegisterComponent();
    Material = UMaterialInstanceDynamic::Create(
        LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/DaoVoTan/Materials/M_Aura")), this);
    Motes->SetMaterial(0, Material);
    Configure(Style);
    SetEmitting(bEmitting);
}
void UAuraComponent::Configure(const FAuraStyle &S)
{
    Style = S;
    Style.Density = FMath::Clamp(S.Density, 8, 96);
    Style.Radius = FMath::Clamp(S.Radius, 30.f, 250.f);
    if (!Motes)
        return;
    Motes->ClearInstances();
    for (int32 I = 0; I < Style.Density; ++I)
        Motes->AddInstance(FTransform::Identity);
    Material->SetVectorParameterValue(TEXT("Tint"), Style.Color);
    Material->SetScalarParameterValue(TEXT("Intensity"), FMath::Clamp(Style.Intensity, 0.f, 5.f));
}
void UAuraComponent::SetEmitting(bool B)
{
    bEmitting = B;
    if (Motes)
        Motes->SetVisibility(B);
    SetComponentTickEnabled(B);
}
void UAuraComponent::TickComponent(float Dt, ELevelTick Tick, FActorComponentTickFunction *Fn)
{
    Super::TickComponent(Dt, Tick, Fn);
    if (!Motes || !bEmitting)
        return;
    Age += Dt;
    for (int32 I = 0; I < Style.Density; ++I)
    {
        float Phase = I * 2.39996f + Age * (.25f + Style.PulseSpeed * .25f);
        float R = Style.Radius * (.85f + .1f * FMath::Sin(Age * Style.PulseSpeed + I));
        float H = FMath::Fmod(I * 7.3f + Age * 22, 170.f) - 85.f;
        Motes->UpdateInstanceTransform(
            I,
            FTransform(FRotator::ZeroRotator, FVector(FMath::Cos(Phase) * R, FMath::Sin(Phase) * R, H),
                       FVector(.025f + .015f * FMath::Sin(Phase) * FMath::Sin(Phase))),
            false, false, true);
    }
    Motes->MarkRenderStateDirty();
}
