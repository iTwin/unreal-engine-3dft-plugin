/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "CoreMinimal.h"

#include "HttpModule.h"

#include "Common/AutoCancelTicker.h"
#include "Common/AutoCancelRequest.h"

struct FCancelRequest
{
	FAutoCancelTicker AutoCancelTicker;
	FAutoCancelRequest AutoCancelRequest;
};

class FITwinServices
{
public:
	struct FExportInfo
	{
		FString Id;
		FString DisplayName;
		FString Status;
		FString iModelId;
		FString ChangesetId;
		FString MeshUrl;
	};

	struct FiTwinInfo
	{
		FString Id;
		FString DisplayName;
		FString Status;
		FString Number;
	};

	struct FiModelInfo
	{
		FString Id;
		FString DisplayName;
		FString Status;
		FString Number;
	};

	struct FChangesetInfo
	{
		FString Id;
		FString DisplayName;
		FString Description;
		int Index;
	};

public:

	static void GetiTwins(FCancelRequest& CancelRequest, std::function<void(TArray<FiTwinInfo> iTwins)> Callback);

	static void GetiTwiniModels(FCancelRequest& CancelRequest, FString iTwinId, std::function<void(TArray<FiModelInfo> iModels)> Callback);

	static void GetiModelChangesets(FCancelRequest& CancelRequest, FString iModelId, std::function<void(TArray<FChangesetInfo> Changesets)> Callback);

	static void GetExportInfo(FCancelRequest& CancelRequest, FString ExportId, std::function<void(FExportInfo ExportInfo)> Callback);

	static void GetExportAndRefresh(FString ExportId, FCancelRequest &CancelRequest, std::function<void(FExportInfo ExportInfo, bool bRefreshUrl)> Callback);

	static void GetExports(FCancelRequest& CancelRequest, std::function<void(TArray<FExportInfo> Exports)> Callback);

	static void GetiModelExports(FCancelRequest& CancelRequest, FString iModelId, FString ChangesetId, std::function<void(TArray<FExportInfo> Exports)> Callback);

	static void StartExport(FCancelRequest& CancelRequest, FString iModelId, FString ChangesetId, std::function<void(FString ExportId)> Callback);

	static void AutoExportAndLoad(FCancelRequest& CancelRequest, FString iModelId, FString ChangesetId, std::function<void(FString ExportId)> Callback);
};

