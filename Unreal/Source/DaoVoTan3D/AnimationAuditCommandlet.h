#pragma once
#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "AnimationAuditCommandlet.generated.h"

/** Loads the explicitly supplied sample mount for inspection; never saves packages. */
UCLASS()
class DAOVOTAN3D_API UAnimationAuditCommandlet : public UCommandlet
{
    GENERATED_BODY()
  public:
    UAnimationAuditCommandlet();
    virtual int32 Main(const FString &Params) override;
};
