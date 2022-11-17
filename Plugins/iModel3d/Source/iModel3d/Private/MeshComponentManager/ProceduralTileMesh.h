/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
