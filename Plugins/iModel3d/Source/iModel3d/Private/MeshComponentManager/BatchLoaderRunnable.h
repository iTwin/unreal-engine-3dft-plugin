/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "CoreMinimal.h"

#include "IModelDecoder.h"
#include "ProceduralTileMesh.h"
#include "Common/SpinLock.h"
#include "GraphicOptions.h"

#include <deque>

class FBatchLoaderRunnable : public FRunnable
{
public:
	FBatchLoaderRunnable(const std::shared_ptr<IModelDecoder>& InModelDecoder);

	virtual uint32 Run() override;

	virtual void Stop() override;

	TSharedPtr<FProceduralTileMesh> PopMesh();

	size_t NumReadyMeshes();

	void SetTriangleBudget(size_t Budget);

	void SetGraphicOptions(const TSharedPtr<FGraphicOptions>& GraphicOptions);

	uint64_t GetElementId(uint32_t ElementIndex);

private:
	bool ReadyTrianglesFull();

private:
	std::deque<TSharedPtr<FProceduralTileMesh>> ReadyRenderableBatches;
	std::atomic_bool bIsRunning = true;
	std::shared_ptr<IModelDecoder> ModelDecoder;
	FSpinLock ReadyRenderableBatchesLock;
	size_t TriangleBudget = 0;
	size_t ReadyTriangles = 0;
	std::unordered_map<uint64_t, uint32_t> ElementIds;
	std::vector<uint64_t> InvElementIds;
	FSpinLock ElementIdsLock;

	FGraphicOptions Options;
	FGraphicOptions UpdatedOptions;
	std::atomic_bool bUpdatedOptions = false;
	FSpinLock OptionsLock;
};
