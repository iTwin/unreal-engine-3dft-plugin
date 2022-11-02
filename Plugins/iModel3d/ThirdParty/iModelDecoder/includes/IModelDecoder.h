#pragma once

#include "IMOptions.h"
#include "MeshTile.h"
#include "StaticString.h"

#include <memory>

class IModelDecoder
{
public:
	virtual void AddBinaryAssets(const uint8_t* Data, size_t Size, const char* RelativePath) = 0;

	virtual StaticString GetPendingRequest() = 0;

	virtual void SetCameraLocation(std::array<float, 3> CamLocation, std::array<float, 3> CamForward) = 0;

	virtual void SetOptions(FIMOptions InOptions) = 0;

	virtual bool IsNextBatchReady() = 0;

	virtual MeshTile* GetNextRenderableBatch() = 0;

	virtual void Release(MeshTile* Object) = 0;

	virtual StaticString GetStatusInfo(float& Percentage) = 0;

	virtual void Release() = 0;
};

IModelDecoder* CreateDecoder();
