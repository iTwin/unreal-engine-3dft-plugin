/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "CoreMinimal.h"

#include "HttpModule.h"

#include "Common/AutoCancelTicker.h"
#include "Common/AutoCancelRequest.h"

class FITwinServices
{
public:
	struct FCancelRequest
	{
		FAutoCancelTicker AutoCancelTicker;
		FAutoCancelRequest AutoCancelRequest;
	};

	struct FExportInfo
	{
		FString iModelId;
		FString ChangesetId;
		FString MeshUrl;
	};

	struct FProjectInfo
	{
		FString Id;
		FString DisplayName;
		FString ProjectNumber;
	};


public:
	static void GetExport(FString ExportId, FString AuthToken, FCancelRequest& CancelRequest, std::function<void(FExportInfo ExportInfo)> Callback);

	static void GetExportAndRefresh(FString ExportId, FCancelRequest &CancelRequest, std::function<void(FExportInfo ExportInfo, bool bRefreshUrl)> Callback);

	static void GetProjects(FCancelRequest& CancelRequest, std::function<void(TArray<FProjectInfo> FITwinServices::Projects)> Callback);
};

