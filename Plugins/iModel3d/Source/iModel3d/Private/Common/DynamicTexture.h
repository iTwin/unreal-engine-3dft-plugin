/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

struct FDynamicTexture
{
private:
	UTexture2D* Texture2D = nullptr;
	int32 Width;
	int32 Height;

public:
	~FDynamicTexture();

	void Initialize(int32 InWidth, int32 InHeight);

	void SetPixels(const TArray<uint8>& Pixels);

	UTexture2D* Get() const { return Texture2D; };
};