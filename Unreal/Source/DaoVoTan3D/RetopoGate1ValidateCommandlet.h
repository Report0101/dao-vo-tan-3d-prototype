#pragma once
#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "RetopoGate1ValidateCommandlet.generated.h"
/** Editor-only, read-only candidate inspection; no gameplay or asset saves. */
UCLASS()
class DAOVOTAN3D_API URetopoGate1ValidateCommandlet : public UCommandlet
{
    GENERATED_BODY()
  public:
    URetopoGate1ValidateCommandlet();
    virtual int32 Main(const FString &Params) override;
};
