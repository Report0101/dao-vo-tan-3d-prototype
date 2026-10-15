#pragma once
#include "Commandlets/Commandlet.h"
#include "CoreMinimal.h"
#include "Gate1AuthorCommandlet.generated.h"
UCLASS()
class DAOVOTAN3D_API UGate1AuthorCommandlet : public UCommandlet
{
    GENERATED_BODY()
  public:
    UGate1AuthorCommandlet();
    virtual int32 Main(const FString &Params) override;
};
