/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "iModelExportService.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "JsonUtilities.h"

#include "Common/APIService.h"
#include "iTwinAuthorizationService.h"

namespace
{
TSharedPtr<FJsonObject> GetChildObject(TSharedPtr<FJsonObject> JsonRoot, FString Element)
{
	TArray<FString> Names;
	Element.ParseIntoArray(Names, TEXT("/"), true);

	for (const auto& Name : Names)
	{
		auto JsonChild = JsonRoot->GetObjectField(Name);
		if (!JsonChild)
		{
			return nullptr;
		}
		JsonRoot = JsonChild;
	}
	return JsonRoot;
}
}

FAPIService::FRequestPtr FIModelExportService::GetExport(FString ExportId, FString AuthToken, std::function<void(FIModelExportService::FExportInfo ExportInfo)> Callback)
{
	FString RequestContent = TEXT("");
	return FAPIService::SendGetRequest(TEXT("https://api.bentley.com/mesh-export/" + ExportId), RequestContent, AuthToken, [Callback](const FString& Response, const FString& ErrorMessage)
	{
		if (!ErrorMessage.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
			return;
		}

		TSharedPtr<FJsonObject> JsonRoot;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response);
		if (!FJsonSerializer::Deserialize(Reader, JsonRoot) || !JsonRoot.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid Reply: invalid Json format!"));
			return;
		}

		auto JsonHref = GetChildObject(JsonRoot, "export/request");
		FString ExportType;
		if (!JsonHref || !JsonHref->TryGetStringField(TEXT("exportType"), ExportType) || ExportType != "3DFT")
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid Reply: Export Type is incorrect!"));
			return;
		}

		FExportInfo ExportInfo;
		if (!JsonHref->TryGetStringField(TEXT("iModelId"), ExportInfo.iModelId))
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid Reply: missing response iModelId field!"));
			return;
		}

		if (!JsonHref->TryGetStringField(TEXT("changesetId"), ExportInfo.ChangesetId))
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid Reply: missing response ChangesetId field!"));
			return;
		}

		JsonHref = GetChildObject(JsonRoot, "export/_links/mesh");
		if (!JsonHref || !JsonHref->TryGetStringField(TEXT("href"), ExportInfo.MeshUrl))
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid Reply: missing response href field!"));
			return;
		}

		Callback(std::move(ExportInfo));
	});
}


void FIModelExportService::GetExportAndRefresh(FString ExportId, FCancelExport& CancelExport, std::function<void(FExportInfo ExportInfo, bool bRefreshUrl)> Callback)
{
	CancelExport.AutoCancelTicker.Reset();
	CancelExport.AutoCancelRequest.Reset();

	CancelExport.AutoCancelTicker = FITwinAuthorizationService::Get().GetAuthTokenAsync([ExportId, Callback, &CancelExport](FString AuthToken)
	{
		CancelExport.AutoCancelRequest = FIModelExportService::GetExport(ExportId, AuthToken, [AuthToken, ExportId, Callback, &CancelExport](FIModelExportService::FExportInfo ExportInfo)
		{
			Callback(ExportInfo, false);
			constexpr auto DelaySeconds = 30 * 60;
			CancelExport.AutoCancelTicker = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([AuthToken, ExportId, Callback, &CancelExport](float Delta) -> bool
			{
				CancelExport.AutoCancelRequest = FIModelExportService::GetExport(ExportId, AuthToken, [Callback](FIModelExportService::FExportInfo ExportInfo)
				{
					Callback(ExportInfo, true);
				});
				return true; // Infinite loop
			}), DelaySeconds * 0.5);
		});
	});
}
