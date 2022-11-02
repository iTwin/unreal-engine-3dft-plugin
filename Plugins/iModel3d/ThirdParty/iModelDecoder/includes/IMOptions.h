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
	float ObjectLoadingSpeed;
	bool bShowObjectsWhileLoading;
	uint32_t RequestsInParallel;

	// Optimization
	uint32_t MaxTrianglesPerBatch;
	float ShadowDistanceCulling;

	// Quality
	FGeometryQuality GeometryQuality;
	bool IgnoreTransparency;
};

constexpr float FPSWhenLoadingMeshes = 60.0;
constexpr float LoadingVerticesPerFrame = 20000;
