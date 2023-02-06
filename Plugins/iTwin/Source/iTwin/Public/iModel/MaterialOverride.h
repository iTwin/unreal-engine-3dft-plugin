/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "CoreMinimal.h"

#include "MaterialOverride.generated.h"

USTRUCT()
struct FMaterialOverride
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
		FColor Color = FColor(0xff, 0xff, 0);

	UPROPERTY(EditAnywhere)
		FString Name;

	UPROPERTY(EditAnywhere)
		bool Enabled = true;

	UPROPERTY(EditAnywhere, Category = "Output Material")
		float Specular = 0.5;

	UPROPERTY(EditAnywhere, Category = "Output Material")
		float Roughness = 0.5;

	UPROPERTY(EditAnywhere, Category = "Output Material")
		float Metalic = 0;

	UPROPERTY(EditAnywhere, Category = "Output Material")
		FColor OutColor = FColor(0xff, 0xff, 0);
};
