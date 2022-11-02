#include "AnimationManager.h"

FAnimationManager::FAnimationManager()
{
}

void FAnimationManager::GetAnimationInfo(int32& Start, int32& End, int32& Duration, bool& Loaded)
{
	Loaded = AnimationData.IsAnimationLoaded();
	if (Loaded)
	{
		AnimationData.GetStartEndTimes(Start, End);
	}
	else
	{
		End = Start = 0;
	}
	Duration = End - Start;
}

void FAnimationManager::SetPlaybackPosition(int32 Time)
{
	auto AnimatedData = AnimationData.GetAnimatedData(Time);
	if (!AnimatedData.Visibility.size())
	{
		return;
	}

	auto Width = 256;
	auto Height = std::ceil(AnimatedData.Visibility.size() / 256.0);

	bool VisibilityChanges = false, ColorChanges = false;
	if (!VisibilityTexture)
	{
		VisibilityTexture = MakeShared<FDynamicTexture>();
		VisibilityTexture->Initialize(Width, Height);

		ColorTexture = MakeShared<FDynamicTexture>();
		ColorTexture->Initialize(Width, Height);

		bTexturesDirty = true;

		AnimationVisibilityPixels.SetNumZeroed(Width * Height * 4);
		AnimationColorPixels.SetNumZeroed(Width * Height * 4);
		VisibilityChanges = ColorChanges = true;
	}

	uint32* ColorPixels = (uint32*)(AnimationColorPixels.GetData());

	for (int i = 0; i < AnimatedData.Visibility.size(); i++)
	{
		if (AnimationVisibilityPixels[i * 4] != AnimatedData.Visibility[i])
		{
			AnimationVisibilityPixels[i * 4] = AnimatedData.Visibility[i];
			VisibilityChanges = true;
		}
		if (ColorPixels[i] != AnimatedData.Color[i])
		{
			ColorPixels[i] = AnimatedData.Color[i];
			ColorChanges = true;
		}
	}

	if (VisibilityChanges)
	{
		VisibilityTexture->SetPixels(AnimationVisibilityPixels);
	}

	if (ColorChanges)
	{
		ColorTexture->SetPixels(AnimationColorPixels);
	}
}
