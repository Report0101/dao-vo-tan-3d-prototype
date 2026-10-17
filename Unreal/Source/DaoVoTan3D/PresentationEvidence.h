#pragma once
#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "HAL/FileManager.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"
// Read the PIE backbuffer explicitly. A global screenshot request can be consumed by an editor
// level viewport instead when both viewports render during unattended automation.
inline bool CapturePIEEvidence(const FString &Filename)
{
    for (const auto &Context : GEngine->GetWorldContexts())
        if (Context.WorldType == EWorldType::PIE)
        {
            auto *Client = Context.World()->GetGameViewport();
            if (!Client || !Client->Viewport)
                return false;
            FViewport *View = Client->Viewport;
            TArray<FColor> Pixels;
            const FIntPoint Size = View->GetSizeXY();
            if (!View->ReadPixels(Pixels))
                return false;
            for (auto &Pixel : Pixels)
                Pixel.A = 255;
            TArray64<uint8> PNG;
            FImageUtils::PNGCompressImageArray(Size.X, Size.Y, Pixels, PNG);
            IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename), true);
            return FFileHelper::SaveArrayToFile(PNG, *Filename);
        }
    return false;
}
#endif
