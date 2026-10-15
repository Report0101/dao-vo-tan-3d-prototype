#include "DaoVoTanGameMode.h"

#include "DaoVoTanCharacter.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ADaoVoTanGameMode::ADaoVoTanGameMode()
{
    DefaultPawnClass = nullptr;
}

void ADaoVoTanGameMode::BeginPlay()
{
    Super::BeginPlay();
    EnsurePrototypeWorld();
    EnsurePlayerPawn();
}

void ADaoVoTanGameMode::EnsurePrototypeWorld()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh)
    {
        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AStaticMeshActor* Ground = World->SpawnActor<AStaticMeshActor>(FVector(0.0f, 0.0f, -50.0f), FRotator::ZeroRotator, Params);
        if (Ground)
        {
            Ground->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
            Ground->SetActorScale3D(FVector(50.0f, 50.0f, 1.0f));
            Ground->SetActorLabel(TEXT("VS1_Ground"));
        }

        const FVector MarkerLocations[] =
        {
            FVector(900.0f, 650.0f, 50.0f),
            FVector(-1100.0f, 500.0f, 80.0f),
            FVector(1300.0f, -900.0f, 120.0f),
            FVector(-1500.0f, -1200.0f, 160.0f)
        };

        for (int32 Index = 0; Index < UE_ARRAY_COUNT(MarkerLocations); ++Index)
        {
            AStaticMeshActor* Marker = World->SpawnActor<AStaticMeshActor>(MarkerLocations[Index], FRotator(0.0f, Index * 23.0f, 0.0f), Params);
            if (Marker)
            {
                Marker->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
                Marker->SetActorScale3D(FVector(2.0f + Index, 2.0f + Index, 2.0f + Index));
                Marker->SetActorLabel(FString::Printf(TEXT("VS1_Landmark_%d"), Index + 1));
            }
        }
    }

    FActorSpawnParameters LightParams;
    LightParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ADirectionalLight* Sun = World->SpawnActor<ADirectionalLight>(FVector::ZeroVector, FRotator(-48.0f, -35.0f, 0.0f), LightParams);
    if (Sun)
    {
        Sun->SetActorLabel(TEXT("VS1_Sun"));
    }

    AExponentialHeightFog* Fog = World->SpawnActor<AExponentialHeightFog>(FVector::ZeroVector, FRotator::ZeroRotator, LightParams);
    if (Fog)
    {
        Fog->SetActorLabel(TEXT("VS1_Fog"));
    }
}

void ADaoVoTanGameMode::EnsurePlayerPawn()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);
    if (!PlayerController || PlayerController->GetPawn())
    {
        return;
    }

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    ADaoVoTanCharacter* Character = World->SpawnActor<ADaoVoTanCharacter>(ADaoVoTanCharacter::StaticClass(), FVector(0.0f, 0.0f, 140.0f), FRotator::ZeroRotator, Params);
    if (Character)
    {
        PlayerController->Possess(Character);
    }
}
