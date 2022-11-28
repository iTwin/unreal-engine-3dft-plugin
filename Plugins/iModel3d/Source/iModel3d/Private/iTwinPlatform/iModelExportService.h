/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "CoreMinimal.h"

#include "HttpModule.h"

#include "Common/AutoCancelTicker.h"
#include "Common/AutoCancelRequest.h"

class FIModelExportService
{
public:
	struct FCancelExport
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

public:
	static TSharedRef<IHttpRequest, ESPMode::ThreadSafe> GetExport(FString ExportId, FString AuthToken, std::function<void(FExportInfo ExportInfo)> Callback);

	static void GetExportAndRefresh(FString ExportId, FCancelExport &CancelExport, std::function<void(FExportInfo ExportInfo, bool bRefreshUrl)> Callback);

	// static void GetExports(FString iModelId);
};

