// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ElementInfo.generated.h"

USTRUCT()
struct FElementInfo
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
		FString Id;

	UPROPERTY(EditAnywhere)
		bool bVisible = true;

	UPROPERTY(EditAnywhere)
		FVector Offset = FVector(0, 0, 0);

	UPROPERTY(EditAnywhere)
		float PixelOffset = 0;
};
