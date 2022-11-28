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
	FString RequestContent = TEXT("");
	CancelRequest.AutoCancelRequest = FAPIService::SendGetRequest(TEXT("https://api.bentley.com/mesh-export/" + ExportId), RequestContent, AuthToken, [Callback](const FString& Response, const FString& ErrorMessage)
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

		JsonHref->TryGetStringField(TEXT("changesetId"), ExportInfo.ChangesetId);

		JsonHref = GetChildObject(JsonRoot, "export/_links/mesh");
		if (!JsonHref || !JsonHref->TryGetStringField(TEXT("href"), ExportInfo.MeshUrl))
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid Reply: missing response href field!"));
			return;
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

void FITwinServices::GetProjects(FCancelRequest& CancelRequest, std::function<void(TArray<FITwinServices::FProjectInfo> Projects)> Callback)
{
	CancelRequest.AutoCancelTicker = FITwinAuthorizationService::Get().GetAuthTokenAsync([Callback, &CancelRequest](FString AuthToken)
	{
		CancelRequest.AutoCancelRequest = FAPIService::SendGetRequest(TEXT("https://api.bentley.com/projects"), "", AuthToken, [this](const FString& Response, const FString& ErrorMessage)
		{
			if (!ErrorMessage.IsEmpty())
			{
				UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
				return;
			}

			//FProjectList ProjectList;
			//FJsonObjectConverter::JsonObjectStringToUStruct<FProjectList>(Response, &ProjectList, 0, 0);
			//ProjectListReceived.Broadcast(std::move(ProjectList));
		});
	});
}
