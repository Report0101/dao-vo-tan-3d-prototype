#include "AnimationAuditCommandlet.h"
#if WITH_EDITOR
#include "AssetRegistry/AssetRegistryModule.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/UObjectHash.h"
#include "UObject/UnrealType.h"
#endif

UAnimationAuditCommandlet::UAnimationAuditCommandlet()
{
    IsClient = true;
    IsServer = false;
    IsEditor = true;
    LogToConsole = true;
}
int32 UAnimationAuditCommandlet::Main(const FString &Params)
{
#if WITH_EDITOR
    FString Root, Output;
    FParse::Value(*Params, TEXT("SampleRoot="), Root);
    FParse::Value(*Params, TEXT("AuditOutput="), Output);
    const bool bDestination = FParse::Param(*Params, TEXT("DestinationOnly"));
    if ((!bDestination && !FPaths::FileExists(Root / TEXT("GameAnimationSample.uproject"))) ||
        Output.IsEmpty())
        return 2;
    if (!bDestination)
        FPackageName::RegisterMountPoint(TEXT("/Game/"), Root / TEXT("Content/"));
    auto &Registry =
        FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
    if (bDestination)
        Registry.ScanPathsSynchronous({TEXT("/Game/DaoVoTan/Locomotion")}, true);
    else
        Registry.ScanPathsSynchronous(
            {TEXT("/Game/Blueprints"), TEXT("/Game/Characters/UEFN_Mannequin")}, true);
    TArray<FAssetData> Assets;
    Registry.GetAssetsByPath(
        bDestination ? TEXT("/Game/DaoVoTan/Locomotion")
                     : TEXT("/Game/Characters/UEFN_Mannequin/Animations/MotionMatchingData"),
        Assets, true);
    TArray<FString> Inspect = {
        TEXT("/Game/Blueprints/SandboxCharacter_CMC_ABP.SandboxCharacter_CMC_ABP"),
        TEXT("/Game/Blueprints/SandboxCharacter_CMC.SandboxCharacter_CMC"),
        TEXT("/Game/Blueprints/ControlRigs/CR_Biped_FootPlacement.CR_Biped_FootPlacement"),
        TEXT("/Game/Characters/UEFN_Mannequin/Meshes/SK_UEFN_Mannequin.SK_UEFN_Mannequin")};
    if (bDestination)
        Inspect.Empty();
    for (const auto &A : Assets)
        Inspect.Add(A.GetObjectPathString());
    TArray<TSharedPtr<FJsonValue>> Records;
    for (const FString &Path : Inspect)
    {
        auto Record = MakeShared<FJsonObject>();
        Record->SetStringField(TEXT("path"), Path);
        UObject *Object = LoadObject<UObject>(nullptr, *Path);
        Record->SetBoolField(TEXT("loaded"), Object != nullptr);
        if (Object)
        {
            Record->SetStringField(TEXT("class"), Object->GetClass()->GetPathName());
            TArray<UObject *> Objects;
            GetObjectsWithPackage(Object->GetPackage(), Objects,
                                  EGetObjectsFlags::IncludeNestedObjects);
            Objects.AddUnique(Object);
            TArray<TSharedPtr<FJsonValue>> Exports;
            for (UObject *Sub : Objects)
            {
                const FString ClassName = Sub->GetClass()->GetName();
                if (Sub != Object && !ClassName.Contains(TEXT("Node")) &&
                    !ClassName.Contains(TEXT("Graph")))
                    continue;
                auto Entry = MakeShared<FJsonObject>();
                Entry->SetStringField(TEXT("object"), Sub->GetPathName());
                Entry->SetStringField(TEXT("class"), Sub->GetClass()->GetPathName());
                auto Properties = MakeShared<FJsonObject>();
                for (TFieldIterator<FProperty> P(Sub->GetClass()); P; ++P)
                {
                    if (P->HasAnyPropertyFlags(CPF_Transient))
                        continue;
                    FString Value;
                    P->ExportTextItem_Direct(Value, P->ContainerPtrToValuePtr<void>(Sub), nullptr,
                                             Sub, PPF_None);
                    if (!Value.IsEmpty() && Value.Len() < 400000)
                        Properties->SetStringField(P->GetName(), Value);
                }
                Entry->SetObjectField(TEXT("properties"), Properties);
                Exports.Add(MakeShared<FJsonValueObject>(Entry));
            }
            Record->SetArrayField(TEXT("exports"), Exports);
        }
        TArray<FName> Dependencies;
        Registry.GetDependencies(FName(*FPackageName::ObjectPathToPackageName(Path)), Dependencies);
        TArray<TSharedPtr<FJsonValue>> Deps;
        for (FName D : Dependencies)
            Deps.Add(MakeShared<FJsonValueString>(D.ToString()));
        Record->SetArrayField(TEXT("dependencies"), Deps);
        Records.Add(MakeShared<FJsonValueObject>(Record));
    }
    auto Report = MakeShared<FJsonObject>();
    Report->SetStringField(TEXT("sourceRoot"), Root);
    Report->SetBoolField(TEXT("readOnlyInspection"), true);
    Report->SetBoolField(TEXT("destinationOnly"), bDestination);
    Report->SetArrayField(TEXT("assets"), Records);
    FString Json;
    FJsonSerializer::Serialize(Report, TJsonWriterFactory<>::Create(&Json));
    return FFileHelper::SaveStringToFile(Json, *Output) ? 0 : 3;
#else
    return 1;
#endif
}
