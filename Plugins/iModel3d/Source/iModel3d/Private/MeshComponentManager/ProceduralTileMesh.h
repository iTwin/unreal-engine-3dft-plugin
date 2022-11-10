#pragma once

#include "CustomProcMeshComponent.h"

struct FTilePosition
{
	FVector Center;
	float Radius;
};

struct FTileMaterial
{
	bool bTranslucent;
	int16 Index;
};

struct FProceduralTileMesh
{
	int32 Id;
	int TriangleNum;
	FTileMaterial Material;
	FTilePosition BatchPosition;
	TArray<TSharedPtr<FCustomProcMeshSection>> ProcMeshes;
	TArray<int32> BatchesToRemove;
	TArray<int32> BatchesToShow;
};
