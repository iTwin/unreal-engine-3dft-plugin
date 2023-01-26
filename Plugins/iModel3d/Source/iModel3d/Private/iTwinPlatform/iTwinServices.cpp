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

struct FiTwinServicesEndPoints
{
	static constexpr auto Server = TEXT("https://api.bentley.com");
	static constexpr auto MeshExport = TEXT("mesh-export");
	static constexpr auto iTwins = TEXT("itwins");
	static constexpr auto iModels = TEXT("imodels");
};

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

FAPIService::FRequestPtr SendGetRequest(const FString& URL, const FString& AuthToken, TArray<FAPIService::FHttpHeader> Headers, TFunction<void(TSharedPtr<FJsonObject>, const FString&)> Callback, FString ApiVersion = "v1")
{
	Headers.Push({ "Authorization", FString::Printf(TEXT("Bearer %s"), *AuthToken) });
	Headers.Push({ "Accept", FString::Printf(TEXT("application/vnd.bentley.itwin-platform.%s+json"), *ApiVersion) });

	return FAPIService::SendGetRequest(URL, "", Headers, Callback);
}

FAPIService::FRequestPtr SendPostRequest(const FString& URL, const FString& AuthToken, const FString& RequestContent, TArray<FAPIService::FHttpHeader> Headers, TFunction<void(TSharedPtr<FJsonObject>, const FString&)> Callback, FString ApiVersion = "v1")
{
	Headers.Push({ "Authorization", FString::Printf(TEXT("Bearer %s"), *AuthToken) });
	Headers.Push({ "Accept", FString::Printf(TEXT("application/vnd.bentley.itwin-platform.%s+json"), *ApiVersion) });

	return FAPIService::SendPostRequest(URL, RequestContent, Headers, Callback);
}

void GetExport(FString ExportId, FString AuthToken, FCancelRequest& CancelRequest, std::function<void(FITwinServices::FExportInfo ExportInfo)> Callback)
{
	auto URL = FString::Printf(TEXT("%s/%s/%s"), FiTwinServicesEndPoints::Server, FiTwinServicesEndPoints::MeshExport, *ExportId);
	CancelRequest.AutoCancelRequest = SendGetRequest(URL, AuthToken, {}, [Callback](TSharedPtr<FJsonObject> JsonRoot, const FString& ErrorMessage)
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

		FITwinServices::FExportInfo ExportInfo = { JsonExport->GetStringField("id"), JsonExport->GetStringField("displayName"), JsonExport->GetStringField("status"),
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

void WaitForTheExportToComplete(FCancelRequest& CancelRequest, FString ExportId, std::function<void(FString ExportId)> Callback)
{
	CancelRequest.AutoCancelTicker = FITwinAuthorizationService::Get().GetAuthTokenAsync([ExportId, Callback, &CancelRequest](FString AuthToken)
	{
		GetExport(ExportId, AuthToken, CancelRequest, [&CancelRequest, Callback, ExportId](auto Export)
		{
			if (Export.Status == "Complete")
			{
				Callback(Export.Id);
			}
			else if (Export.Status != "Invalid")
			{
				CancelRequest.AutoCancelTicker = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Callback, ExportId, &CancelRequest](float Delta) -> bool
				{
					WaitForTheExportToComplete(CancelRequest, ExportId, Callback);
					return false;
				}), 2500); // 2.5s
			}
			else
			{
				Callback("");
			}
		});
	});
}
}

void FITwinServices::GetiTwins(FCancelRequest& CancelRequest, std::function<void(TArray<FiTwinInfo> iTwins)> Callback)
{
	CancelRequest.AutoCancelTicker = FITwinAuthorizationService::Get().GetAuthTokenAsync([Callback, &CancelRequest](FString AuthToken)
	{
		auto URL = FString::Printf(TEXT("%s/%s/?subClass=Project&status=Active"), FiTwinServicesEndPoints::Server, FiTwinServicesEndPoints::iTwins);
		CancelRequest.AutoCancelRequest = SendGetRequest(URL, AuthToken, {}, [Callback](TSharedPtr<FJsonObject> JsonRoot, const FString& ErrorMessage)
		{
			if (!ErrorMessage.IsEmpty())
			{
				UE_LOG(LogTemp, Error, TEXT("Invalid Reply: %s"), *ErrorMessage);
				return;
			}

			auto JsonExports = JsonRoot->GetArrayField("iTwins");
			TArray<FiTwinInfo> iTwins;
			for (const auto JsonExport : JsonExports)
			{
				const auto JsonObject = JsonExport->AsObject();
				FiTwinInfo iTwinInfo = { JsonObject->GetStringField("id"), JsonObject->GetStringField("displayName"), JsonObject->GetStringField("status"), JsonObject->GetStringField("number") };
				iTwins.Push(iTwinInfo);
			}
			Callback(iTwins);
		});
	});
}

void FITwinServices::GetiTwiniModels(FCancelRequest& CancelRequest, FString iTwinId, std::function<void(TArray<FiModelInfo> iModels)> Callback)
{
	CancelRequest.AutoCancelTicker = FITwinAuthorizationService::Get().GetAuthTokenAsync([Callback, &CancelRequest, iTwinId](FString AuthToken)
	{
		auto URL = FString::Printf(TEXT("%s/%s/?iTwinId=%s"), FiTwinServicesEndPoints::Server, FiTwinServicesEndPoints::iModels, *iTwinId);
		CancelRequest.AutoCancelRequest = SendGetRequest(URL, AuthToken, {}, [Callback](TSharedPtr<FJsonObject> JsonRoot, const FString& ErrorMessage)
		{
			if (!ErrorMessage.IsEmpty())
			{
				UE_LOG(LogTemp, Error, TEXT("Invalid Reply: %s"), *ErrorMessage);
				return;
			}

			auto JsonExports = JsonRoot->GetArrayField("iModels");
			TArray<FiModelInfo> iModels;
			for (const auto JsonExport : JsonExports)
			{
				const auto JsonObject = JsonExport->AsObject();
				FiModelInfo iModel = { JsonObject->GetStringField("id"), JsonObject->GetStringField("displayName") };
				iModels.Push(iModel);
			}
			Callback(iModels);
		}, "v2");
	});
}

void FITwinServices::GetiModelChangesets(FCancelRequest& CancelRequest, FString iModelId, std::function<void(TArray<FChangesetInfo> Changesets)> Callback)
{
	auto URL = FString::Printf(TEXT("%s/%s/%s/changesets?$orderBy=index%%20desc"), FiTwinServicesEndPoints::Server, FiTwinServicesEndPoints::iModels, *iModelId);
	CancelRequest.AutoCancelTicker = FITwinAuthorizationService::Get().GetAuthTokenAsync([Callback, &CancelRequest, URL](FString AuthToken)
	{
		CancelRequest.AutoCancelRequest = SendGetRequest(URL, AuthToken, {}, [Callback](TSharedPtr<FJsonObject> JsonRoot, const FString& ErrorMessage)
		{
			if (!ErrorMessage.IsEmpty())
			{
				UE_LOG(LogTemp, Error, TEXT("Invalid Reply: %s"), *ErrorMessage);
				return;
			}

			auto JsonExports = JsonRoot->GetArrayField("changesets");
			TArray<FChangesetInfo> Changesets;
			for (const auto JsonExport : JsonExports)
			{
				const auto JsonObject = JsonExport->AsObject();
				FChangesetInfo Changeset = { JsonObject->GetStringField("id"), JsonObject->GetStringField("displayName"), JsonObject->HasField("description") ? JsonObject->GetStringField("description") : "", JsonObject->GetIntegerField("index")};
				Changesets.Push(Changeset);
			}
			Callback(Changesets);
		});
	});
}

void FITwinServices::GetExportInfo(FCancelRequest& CancelRequest, FString ExportId, std::function<void(FITwinServices::FExportInfo ExportInfo)> Callback)
{
	CancelRequest.AutoCancelTicker = FITwinAuthorizationService::Get().GetAuthTokenAsync([Callback, &CancelRequest, ExportId](FString AuthToken)
	{
		GetExport(ExportId, AuthToken, CancelRequest, Callback);
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
		auto URL = FString::Printf(TEXT("%s/%s/"), FiTwinServicesEndPoints::Server, FiTwinServicesEndPoints::MeshExport);
		CancelRequest.AutoCancelRequest = SendGetRequest(URL, AuthToken, {}, [Callback](TSharedPtr<FJsonObject> JsonRoot, const FString& ErrorMessage)
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

void FITwinServices::GetiModelExports(FCancelRequest& CancelRequest, FString iModelId, FString ChangesetId, std::function<void(TArray<FExportInfo> Exports)> Callback)
{
	auto URL = FString::Printf(TEXT("%s/%s/?iModelId=%s&changesetId=%s"), FiTwinServicesEndPoints::Server, FiTwinServicesEndPoints::MeshExport, *iModelId, *ChangesetId);
	CancelRequest.AutoCancelTicker = FITwinAuthorizationService::Get().GetAuthTokenAsync([Callback, &CancelRequest, URL](FString AuthToken)
	{
		CancelRequest.AutoCancelRequest = SendGetRequest(URL, AuthToken, { { "Prefer", "return=representation" } }, [Callback](TSharedPtr<FJsonObject> JsonRoot, const FString& ErrorMessage)
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

				auto JsonHref = JsonObject->GetObjectField("request");
				if (JsonHref && JsonHref->GetStringField(TEXT("exportType")) == "3DFT")
				{
					FExportInfo Export = { JsonObject->GetStringField("id"), JsonObject->GetStringField("displayName"), JsonObject->GetStringField("status") };
					Exports.Push(Export);
				}
			}
			Callback(Exports);
		});
	});
}

void FITwinServices::StartExport(FCancelRequest& CancelRequest, FString iModelId, FString ChangesetId, std::function<void(FString ExportId)> Callback)
{
	auto URL = FString::Printf(TEXT("%s/%s/"), FiTwinServicesEndPoints::Server, FiTwinServicesEndPoints::MeshExport);
	auto RequestContent = FString::Printf(TEXT("{\"iModelId\":\"%s\",\"changesetId\":\"%s\",\"exportType\":\"3DFT\",\"geometryOptions\":{}}"), *iModelId, *ChangesetId);
	CancelRequest.AutoCancelTicker = FITwinAuthorizationService::Get().GetAuthTokenAsync([Callback, &CancelRequest, URL, RequestContent](FString AuthToken)
	{
		CancelRequest.AutoCancelRequest = SendPostRequest(URL, AuthToken, RequestContent, {{"Content-Type", "application/json"}}, [Callback](TSharedPtr<FJsonObject> Response, const FString& ErrorMessage)
		{
			if (!ErrorMessage.IsEmpty())
			{
				UE_LOG(LogTemp, Error, TEXT("Invalid Reply: %s"), *ErrorMessage);
			}
			else
			{
				if (Response->HasField(TEXT("error")))
				{
					UE_LOG(LogTemp, Error, TEXT("Invalid Reply: %s"), *Response->GetObjectField("error")->GetStringField("message"));
				}
				else
				{
					auto JsonExport = Response->GetObjectField("export");
					if (!JsonExport)
					{
						UE_LOG(LogTemp, Error, TEXT("Invalid Reply: Export not defined."));
					}
					else
					{
						Callback(JsonExport->GetStringField("id"));
					}
				}
			}
		});
	});
}

void FITwinServices::AutoExportAndLoad(FCancelRequest& CancelRequest, FString iModelId, FString ChangesetId, std::function<void(FString ExportId)> Callback)
{
	GetiModelExports(CancelRequest, iModelId, ChangesetId, [&CancelRequest, Callback, iModelId, ChangesetId](auto Exports)
	{
		for (const auto& Export : Exports)
		{
			// Export.Id
			if (Export.Status == "Complete")
			{
				return Callback(Export.Id);
			}
			else if (Export.Status != "Invalid")
			{
				return WaitForTheExportToComplete(CancelRequest, Export.Id, Callback);
				return;
			}
		}
		StartExport(CancelRequest, iModelId, ChangesetId, [&CancelRequest, Callback](auto ExportId)
		{
			WaitForTheExportToComplete(CancelRequest, ExportId, Callback);
		});
	});
}

void FITwinServices::CheckAuthorization(FCancelRequest& CancelRequest, std::function<void(bool bSuccess, FString Error)> Callback)
{
	CancelRequest.AutoCancelTicker = FITwinAuthorizationService::Get().GetAuthTokenAsync([Callback, &CancelRequest](FString AuthToken)
	{
		if (AuthToken == "")
		{
			Callback(false, FITwinAuthorizationService::Get().GetLastError());
		}
		else
		{
			Callback(true, "");
		}
	});
}
