#pragma once
#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "AnimationAuthorCommandlet.generated.h"
UCLASS()
class DAOVOTAN3D_API UAnimationAuthorCommandlet : public UCommandlet
{
    GENERATED_BODY()
  public:
    UAnimationAuthorCommandlet();
    virtual int32 Main(const FString &Params) override;
};
