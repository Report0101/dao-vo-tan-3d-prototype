#include "PresentationRecipe.h"
void UPresentationRecipe::ApplyAppearanceSeed(const FString &Seed)
{
    const uint32 Token = uint32(FCString::Strtoui64(*Seed.Left(8), nullptr, 16));
    HeadVariant = Token % 3;
    HairVariant = (Token / 3) % 4;
    GarmentVariant = (Token / 12) % 3;
    AccessoryVariant = (Token / 36) % 2;
    bAccessory = true;
    const FLinearColor Skins[] = {FLinearColor(.63f, .39f, .24f), FLinearColor(.43f, .24f, .14f),
                                  FLinearColor(.72f, .49f, .32f)};
    SkinColor = Skins[(Token / 72) % 3];
    HairColor = (Token & 256) ? FLinearColor(.12f, .15f, .17f) : FLinearColor(.012f, .017f, .022f);
    FactionColor = TrimColor;
}
