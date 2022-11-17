/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <vector>

struct FGeometryQuality
{
	uint8_t Near;
	uint8_t Far;
};

struct FIMOptions
{
	// Loading
	uint32_t RequestsInParallel;

	// Optimization
	uint32_t MaxTrianglesPerBatch;

	// Quality
	FGeometryQuality GeometryQuality;
	bool IgnoreTransparency;
};

constexpr float FPSWhenLoadingMeshes = 60.0;
constexpr float LoadingVerticesPerFrame = 20000;
