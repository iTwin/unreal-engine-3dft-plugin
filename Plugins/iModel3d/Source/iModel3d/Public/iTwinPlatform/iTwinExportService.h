/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "iTwinExportService.generated.h"

USTRUCT(BlueprintType)
struct FMeshExport
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(BlueprintReadOnly)
		FString iModelId;

	UPROPERTY(BlueprintReadOnly)
		FString ChangesetId;

	UPROPERTY(BlueprintReadOnly)
		FString MeshUrl;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOniTwinExportReceived, FMeshExport, Export);

UCLASS(BlueprintType)
class IMODEL3D_API UiTwinExportService : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
		FOniTwinExportReceived MeshExportReceived;

	UFUNCTION(BlueprintCallable)
		void GetExport(FString ExportId, const FString AuthToken);
};

