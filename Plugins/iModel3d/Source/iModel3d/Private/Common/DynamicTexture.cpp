#include "DynamicTexture.h"

#include "Rendering/Texture2DResource.h"

namespace
{
	// Send the render command to update the texture
	void UpdateTextureRegions(UTexture2D* Texture2D, uint32 Width, uint32 Height, TSharedPtr<TArray<uint8>> Pixels)
	{
		if (!Texture2D || !Texture2D->GetResource())
		{
			return;
		}

		ENQUEUE_RENDER_COMMAND(UpdateTextureDataCommand)([Texture2D, Width, Height, Pixels](FRHICommandListImmediate& RHICmdList)
		{
			auto Region = FUpdateTextureRegion2D(0, 0, 0, 0, Texture2D->GetSizeX(), Texture2D->GetSizeY());
			auto SrcBpp = 4;
			auto Pitch = Texture2D->GetSizeX() * SrcBpp;
			auto RHITexture = ((FTexture2DResource*)Texture2D->GetResource())->GetTexture2DRHI();
			auto MipIndex = 0;

			RHIUpdateTexture2D(RHITexture, MipIndex, Region, Pitch, Pixels->GetData() + Region.SrcY * Pitch + Region.SrcX * SrcBpp);
		});
	}
}

FDynamicTexture::~FDynamicTexture()
{
	if (Texture2D && Texture2D->IsValidLowLevel())
	{
		Texture2D->RemoveFromRoot();
	}
}

void FDynamicTexture::Initialize(int32 InWidth, int32 InHeight)
{
	Width = InWidth;
	Height = InHeight;

	if (!Width || !Height)
	{
		return;
	}

	Texture2D = UTexture2D::CreateTransient(Width, Height);

	// Turn off Gamma correction
	Texture2D->SRGB = 0;
	// Make sure it never gets garbage collected
	Texture2D->AddToRoot();
	// Update the texture with these new settings
	Texture2D->UpdateResource();

	// Ensure there's no compression (we're editing pixel-by-pixel)
	//Texture2D->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
}

void FDynamicTexture::SetPixels(const TArray<uint8> &Pixels)
{
	UpdateTextureRegions(Texture2D, Width, Height, MakeShared<TArray<uint8>>(Pixels));
}
