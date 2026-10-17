#include "AuraComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
UAuraComponent::UAuraComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}
void UAuraComponent::BeginPlay()
{
    Super::BeginPlay();
    Motes = NewObject<UNiagaraComponent>(GetOwner());
    GetOwner()->AddInstanceComponent(Motes);
    Motes->SetupAttachment(this);
    Motes->SetAutoActivate(false);
    Motes->SetAsset(
        LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/DaoVoTan/Visual11/NS_CultivationAura")));
    Motes->SetCastShadow(false);
    Motes->RegisterComponent();
    Material = UMaterialInstanceDynamic::Create(
        LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/DaoVoTan/Visual11/M_QiSprite")), this);
    Configure(Style);
    SetEmitting(bEmitting);
}
void UAuraComponent::Configure(const FAuraStyle &S)
{
    Style = S;
    Style.Density = FMath::Clamp(S.Density, 8, 96);
    Style.Radius = FMath::Clamp(S.Radius, 30.f, 250.f);
    if (!Motes || !Material)
        return;
    Material->SetVectorParameterValue(TEXT("Tint"), Style.Color);
    Material->SetScalarParameterValue(TEXT("Intensity"), FMath::Clamp(Style.Intensity, 0.f, 5.f));
    Motes->SetVariableMaterial(TEXT("User.AuraMaterial"), Material);
    Motes->SetVariableLinearColor(TEXT("User.Color"), Style.Color);
    Motes->SetVariableFloat(TEXT("User.Density"), float(Style.Density));
    Motes->SetVariableFloat(TEXT("User.Radius"), Style.Radius);
    Motes->SetVariableFloat(TEXT("User.VerticalMotion"), Style.VerticalMotion);
    Motes->SetVariableFloat(TEXT("User.Intensity"), Style.Intensity);
    Motes->SetVariableFloat(TEXT("User.Pulse"), Style.PulseSpeed);
    Motes->SetRelativeScale3D(FVector(1, 1, .85f));
}
void UAuraComponent::SetEmitting(bool B)
{
    const bool Changed = bEmitting != B;
    bEmitting = B;
    if (Motes)
    {
        if (B)
        {
            if (Changed || !Motes->IsActive())
                Motes->Activate(true);
        }
        else
            Motes->DeactivateImmediate();
    }
    SetComponentTickEnabled(B);
}
void UAuraComponent::TickComponent(float Dt, ELevelTick Tick, FActorComponentTickFunction *Fn)
{
    Super::TickComponent(Dt, Tick, Fn);
    if (!Motes || !bEmitting)
        return;
    Age += Dt;
    Material->SetScalarParameterValue(
        TEXT("Intensity"), Style.Intensity * (.78f + .22f * FMath::Sin(Age * Style.PulseSpeed)));
    // Cosmetic breathing and ground-relative rise; Niagara owns all sprite particles.
    Motes->SetRelativeLocation(
        FVector(0, 0, -60 + FMath::Sin(Age * .6f) * Style.VerticalMotion * .2f));
    Motes->SetRelativeRotation(FRotator(0, Age * 9, 0));
}
