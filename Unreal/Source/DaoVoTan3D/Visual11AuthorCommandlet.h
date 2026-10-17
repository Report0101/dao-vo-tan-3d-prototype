#pragma once
#include "Commandlets/Commandlet.h"
#include "CoreMinimal.h"
#include "Visual11AuthorCommandlet.generated.h"
/** Reproducible original presentation assets; never called in gameplay. */
UCLASS()
class DAOVOTAN3D_API UVisual11AuthorCommandlet : public UCommandlet
{
    GENERATED_BODY()
  public:
    UVisual11AuthorCommandlet();
    virtual int32 Main(const FString &Params) override;
};
