#pragma once
#include "CoreMinimal.h"
#if WITH_EDITOR
class UAnimSequence;
/** Single editor-only source for the project's jian poses and landing search interval. */
namespace PresentationAnimationAuthoring
{
bool BakeJian();
FFloatInterval LandingImpactRange(UAnimSequence *Clip);
} // namespace PresentationAnimationAuthoring
#endif
