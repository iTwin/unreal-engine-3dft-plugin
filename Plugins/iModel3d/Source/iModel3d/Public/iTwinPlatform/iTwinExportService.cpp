#include "iTwinExportService.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "JsonUtilities.h"

#include "Common/APIService.h"

TSharedPtr<FJsonObject> GetChildObject(TSharedPtr<FJsonObject> JsonRoot, FString Element)
{
	TArray<FString> Names;
	Element.ParseIntoArray(Names, TEXT("/"), true);

	for (const auto &Name : Names)
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

void UiTwinExportService::GetExport(FString ExportId, const FString AuthToken)
{
	FString RequestContent = TEXT("");
	FAPIService::SendGetRequest(TEXT("https://api.bentley.com/mesh-export/" + ExportId), RequestContent, AuthToken, [this](const FString& Response, const FString& ErrorMessage)
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

		FMeshExport MeshUrl;
		if (!JsonHref->TryGetStringField(TEXT("iModelId"), MeshUrl.iModelId))
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid Reply: missing response iModelId field!"));
			return;
		}

		if (!JsonHref->TryGetStringField(TEXT("changesetId"), MeshUrl.ChangesetId))
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid Reply: missing response ChangesetId field!"));
			return;
		}

		JsonHref = GetChildObject(JsonRoot, "export/_links/mesh");
		if (!JsonHref || !JsonHref->TryGetStringField(TEXT("href"), MeshUrl.MeshUrl))
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid Reply: missing response href field!"));
			return;
		}

		MeshExportReceived.Broadcast(MeshUrl);
	});
}
