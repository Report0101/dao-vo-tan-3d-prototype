#include "ValleyStage.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
static void Add(UHierarchicalInstancedStaticMeshComponent *M, FVector P, FVector S,
                FRotator R = FRotator::ZeroRotator)
{
    M->AddInstance(FTransform(R, P, S));
}
AValleyStage::AValleyStage()
{
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    Rocks = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("Rocks"));
    Trunks = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("Trunks"));
    Leaves = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("Leaves"));
    Stone = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("AltarAndPath"));
    Wood = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("Gate"));
    Gold = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("AltarTrim"));
    for (auto *C : {Rocks.Get(), Trunks.Get(), Leaves.Get(), Stone.Get(), Wood.Get(), Gold.Get()})
    {
        C->SetupAttachment(RootComponent);
        C->SetCollisionProfileName(TEXT("BlockAll"));
    }
}
void AValleyStage::OnConstruction(const FTransform &T)
{
    Super::OnConstruction(T);
    auto Setup = [](UHierarchicalInstancedStaticMeshComponent *C, const TCHAR *Mesh, const TCHAR *Mat)
    {
        C->ClearInstances();
        C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, Mesh));
        C->SetMaterial(0, LoadObject<UMaterialInterface>(nullptr, Mat));
    };
    Setup(Rocks, TEXT("/Engine/BasicShapes/Sphere.Sphere"), TEXT("/Game/DaoVoTan/Materials/M_Rock"));
    Setup(Trunks, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"), TEXT("/Game/DaoVoTan/Materials/M_Wood"));
    Setup(Leaves, TEXT("/Engine/BasicShapes/Cone.Cone"), TEXT("/Game/DaoVoTan/Materials/M_Leaf"));
    Setup(Stone, TEXT("/Engine/BasicShapes/Cube.Cube"), TEXT("/Game/DaoVoTan/Materials/M_Stone"));
    Setup(Wood, TEXT("/Engine/BasicShapes/Cube.Cube"), TEXT("/Game/DaoVoTan/Materials/M_Wood"));
    Setup(Gold, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"), TEXT("/Game/DaoVoTan/Materials/M_Gold"));
    FRandomStream Rand(4821);
    for (int32 I = 0; I < 130; ++I)
    {
        float X = Rand.FRandRange(-17500, 17500), Y = Rand.FRandRange(3500, 11000) * (I % 2 ? 1 : -1);
        float Z = FMath::Square(FMath::Max(0.f, FMath::Abs(Y) - 2700)) * 0.000065f;
        Add(Rocks, FVector(X, Y, Z + 400),
            FVector(Rand.FRandRange(8, 18), Rand.FRandRange(7, 15), Rand.FRandRange(15, 40)),
            FRotator(Rand.FRandRange(-25, 25), Rand.FRandRange(0, 360), 0));
    }
    for (int32 I = 0; I < 100; ++I)
    {
        float X = Rand.FRandRange(-17000, 17000), Y = Rand.FRandRange(1100, 4100) * (I % 2 ? 1 : -1);
        float Z = FMath::Square(FMath::Max(0.f, FMath::Abs(Y) - 2700)) * 0.000065f;
        Add(Trunks, FVector(X, Y, Z + 420), FVector(.9f, .9f, 8.4f));
        for (int32 J = 0; J < 3; ++J)
            Add(Leaves, FVector(X, Y, Z + 750 + J * 220), FVector(7 - J * 1.3f, 7 - J * 1.3f, 7));
    }
    for (int32 I = 0; I < 65; ++I)
    {
        float X = -15000 + I * 470;
        Add(Stone, FVector(X, 120 * FMath::Sin(I * .2f), 0), FVector(3.7f, 7, .25f),
            FRotator(0, Rand.FRandRange(-5, 5), 0));
    }
    // Compact altar, original gate silhouette and destination terrace.
    Add(Gold, FVector(-11100, 0, 8), FVector(5, 5, .20f));
    Add(Stone, FVector(-11100, 0, 20), FVector(3.3f, 3.3f, .3f));
    for (float Y : {-480.f, 480.f})
    {
        Add(Wood, FVector(-13400, Y, 360), FVector(.65f, .65f, 7.2f));
        Add(Stone, FVector(-13400, Y, 30), FVector(1.3f, 1.3f, .6f));
    }
    Add(Wood, FVector(-13400, 0, 720), FVector(1, 12, .6f));
    Add(Wood, FVector(-13400, 0, 825), FVector(1.3f, 13, .4f));
    Add(Stone, FVector(13000, 0, 80), FVector(14, 14, 1.6f));
    Add(Gold, FVector(13000, 0, 170), FVector(8, 8, .2f));
}
