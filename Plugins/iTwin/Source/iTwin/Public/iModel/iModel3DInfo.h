/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "CoreMinimal.h"

#include "iModel3DInfo.generated.h"

USTRUCT(BlueprintType)
struct FiModel3DInfo
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
		FVector BoundingBoxMin = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadOnly)
		FVector BoundingBoxMax = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadOnly)
		FVector ModelCenter = FVector(0, 0, 0);
};
