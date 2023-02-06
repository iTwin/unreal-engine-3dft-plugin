/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Common/DynamicTexture.h"
#include "ModelAnimationData.h"

class FAnimationManager
{
public:
	FAnimationManager();

	void GetAnimationInfo(int32& Start, int32& End, int32& Duration, bool& Loaded);
	void SetPlaybackPosition(int32 Time);
	bool AreTexturesDirty() { return bTexturesDirty; };
	void ClearTexturesDirty() { bTexturesDirty = false; };

public:
	TSharedPtr<FDynamicTexture> VisibilityTexture;
	TSharedPtr<FDynamicTexture> ColorTexture;

private:
	FModelAnimationData AnimationData;
	TArray<uint8> AnimationVisibilityPixels;
	TArray<uint8> AnimationColorPixels;
	bool bTexturesDirty = false;
};
