#pragma once
#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "PlayFeelAuditCommandlet.generated.h"
/** Read-only measurement of project-owned sword geometry and animation poses. */
UCLASS()
class DAOVOTAN3D_API UPlayFeelAuditCommandlet : public UCommandlet
{
    GENERATED_BODY()
  public:
    UPlayFeelAuditCommandlet();
    virtual int32 Main(const FString &Params) override;
};
