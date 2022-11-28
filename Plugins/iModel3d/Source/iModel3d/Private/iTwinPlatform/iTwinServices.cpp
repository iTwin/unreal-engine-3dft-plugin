/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "iTwinServices.h"

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

void FITwinServices::GetExport(FString ExportId, FString AuthToken, FCancelRequest& CancelRequest, std::function<void(FITwinServices::FExportInfo ExportInfo)> Callback)
{
	CancelRequest.AutoCancelRequest = FAPIService::SendGetRequest(TEXT("https://api.bentley.com/mesh-export/" + ExportId), "", AuthToken, [Callback](TSharedPtr<FJsonObject> JsonRoot, const FString& ErrorMessage)
	{
		if (!ErrorMessage.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid Reply: %s"), *ErrorMessage);
			return;
		}

		auto JsonExport = JsonRoot->GetObjectField("export");
		if (!JsonExport)
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid Reply: Export not defined."));
			return;
		}

		auto JsonHref = GetChildObject(JsonRoot, "export/request");
		if (!JsonHref)
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid Reply: Export request not defined."));
			return;
		}

		if (JsonHref->GetStringField(TEXT("exportType")) != "3DFT")
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid Reply: Export Type is incorrect!"));
			return;
		}

		FExportInfo ExportInfo = { JsonExport->GetStringField("id"), JsonExport->GetStringField("displayName"), JsonExport->GetStringField("status"),
									JsonHref->GetStringField(TEXT("iModelId")), JsonHref->GetStringField(TEXT("changesetId")) };

		if (ExportInfo.Status == "Complete")
		{
			JsonHref = GetChildObject(JsonRoot, "export/_links/mesh");
			if (!JsonHref || !JsonHref->TryGetStringField(TEXT("href"), ExportInfo.MeshUrl))
			{
				UE_LOG(LogTemp, Error, TEXT("Invalid Reply: missing response href field!"));
				return;
			}
		}

		Callback(std::move(ExportInfo));
	});
}

void FITwinServices::GetExportAndRefresh(FString ExportId, FCancelRequest& CancelRequest, std::function<void(FExportInfo ExportInfo, bool bRefreshUrl)> Callback)
{
	CancelRequest.AutoCancelTicker.Reset();
	CancelRequest.AutoCancelRequest.Reset();

	CancelRequest.AutoCancelTicker = FITwinAuthorizationService::Get().GetAuthTokenAsync([ExportId, Callback, &CancelRequest](FString AuthToken)
	{
		GetExport(ExportId, AuthToken, CancelRequest, [AuthToken, ExportId, Callback, &CancelRequest](FExportInfo ExportInfo)
		{
			Callback(ExportInfo, false);
			constexpr auto DelaySeconds = 30 * 60;
			CancelRequest.AutoCancelTicker = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([AuthToken, ExportId, Callback, &CancelRequest](float Delta) -> bool
			{
				GetExport(ExportId, AuthToken, CancelRequest, [Callback](FExportInfo ExportInfo)
				{
					Callback(ExportInfo, true);
				});
				return true; // Infinite loop
			}), DelaySeconds * 0.5);
		});
	});
}

void FITwinServices::GetExports(FCancelRequest& CancelRequest, std::function<void(TArray<FExportInfo> Exports)> Callback)
{
	CancelRequest.AutoCancelTicker = FITwinAuthorizationService::Get().GetAuthTokenAsync([Callback, &CancelRequest](FString AuthToken)
	{
		CancelRequest.AutoCancelRequest = FAPIService::SendGetRequest(TEXT("https://api.bentley.com/mesh-export/"), "", AuthToken, [Callback](TSharedPtr<FJsonObject> JsonRoot, const FString& ErrorMessage)
		{
			if (!ErrorMessage.IsEmpty())
			{
				UE_LOG(LogTemp, Error, TEXT("Invalid Reply: %s"), *ErrorMessage);
				return;
			}

			auto JsonExports = JsonRoot->GetArrayField("exports");
			TArray<FExportInfo> Exports;
			for (const auto JsonExport : JsonExports)
			{
				const auto JsonObject = JsonExport->AsObject();
				FExportInfo Export = { JsonObject->GetStringField("id"), JsonObject->GetStringField("displayName"), JsonObject->GetStringField("status") };
				Exports.Push(Export);
			}
			Callback(Exports);
		});
	});
}
