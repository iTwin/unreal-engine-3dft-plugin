/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "iTwinWebServices_Info.generated.h"

USTRUCT(BlueprintType)
struct FExportInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "ExportInfo")
		FString Id;

	UPROPERTY(BlueprintReadOnly, Category = "ExportInfo")
		FString DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "ExportInfo")
		FString Status;

	UPROPERTY(BlueprintReadOnly, Category = "ExportInfo")
		FString iModelId;

	UPROPERTY(BlueprintReadOnly, Category = "ExportInfo")
		FString ChangesetId;

	UPROPERTY(BlueprintReadOnly, Category = "ExportInfo")
		FString MeshUrl;
};

USTRUCT(BlueprintType)
struct FExportInfos
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(BlueprintReadOnly, Category = "ExportInfo")
		TArray<FExportInfo> ExportInfos;
};

USTRUCT(BlueprintType)
struct FiTwinInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "iTwinInfo")
		FString Id;

	UPROPERTY(BlueprintReadOnly, Category = "iTwinInfo")
		FString DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "iTwinInfo")
		FString Status;

	UPROPERTY(BlueprintReadOnly, Category = "iTwinInfo")
		FString Number;
};

USTRUCT(BlueprintType)
struct FiTwinInfos
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "iTwinInfo")
		TArray<FiTwinInfo> iTwins;
};

USTRUCT(BlueprintType)
struct FiModelInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "iModelInfo")
		FString Id;

	UPROPERTY(BlueprintReadOnly, Category = "iModelInfo")
		FString DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "iModelInfo")
		FString Status;

	UPROPERTY(BlueprintReadOnly, Category = "iModelInfo")
		FString Number;
};

USTRUCT(BlueprintType)
struct FiModelInfos
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "iModelInfo")
		TArray<FiModelInfo> iModels;
};

USTRUCT(BlueprintType)
struct FChangesetInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "ChangesetInfo")
		FString Id;

	UPROPERTY(BlueprintReadOnly, Category = "ChangesetInfo")
		FString DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "ChangesetInfo")
		FString Description;

	UPROPERTY(BlueprintReadOnly, Category = "ChangesetInfo")
		int Index;
};

USTRUCT(BlueprintType)
struct FChangesetInfos
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "ChangesetInfo")
		TArray<FChangesetInfo> Cahngesets;
};

USTRUCT(BlueprintType)
struct FSavedViewInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "SavedViewInfo")
		FString Id;

	UPROPERTY(BlueprintReadOnly, Category = "SavedViewInfo")
		FString DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "SavedViewInfo")
		bool bShared;
};

USTRUCT(BlueprintType)
struct FSavedViewInfos
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "SavedView")
		TArray<FSavedViewInfo> SavedViews;
};

USTRUCT(BlueprintType)
struct FSavedView
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "SavedView")
		FVector Origin;

	UPROPERTY(BlueprintReadOnly, Category = "SavedView")
		FVector Extents;

	UPROPERTY(BlueprintReadOnly, Category = "SavedView")
		FRotator Angles;
};
