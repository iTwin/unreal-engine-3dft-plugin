#include "iTwinWebServices/iTwinWebServices.h"
#include "iTwinPlatform/iTwinServices.h"

UiTwinWebServices::UiTwinWebServices()
{
	CancelRequest = std::make_shared<FCancelRequest>();
}

void UiTwinWebServices::GetiTwins()
{
	FITwinServices::GetiTwins(*CancelRequest, [this](auto iTwins) {
		TArray<FiTwinInfo> copiedArray;
		for (const auto& info : iTwins)
		{
			copiedArray.Add({ info.Id, info.DisplayName, info.Status, info.Number });
		}
		this->OnGetiTwinsComplete.Broadcast(true, { std::move(copiedArray) });
	});
}

void UiTwinWebServices::GetiTwiniModels(FString iTwinId)
{
	FITwinServices::GetiTwiniModels(*CancelRequest, iTwinId, [this](auto iModels) {
		TArray<FiModelInfo> copiedArray;
		for (const auto& info : iModels)
		{
			copiedArray.Add({ info.Id, info.DisplayName, info.Status, info.Number });
		}
		this->OnGetiTwiniModelsComplete.Broadcast(true, { std::move(copiedArray) });
	});
}

void UiTwinWebServices::GetiModelChangesets(FString iModelId)
{
	FITwinServices::GetiModelChangesets(*CancelRequest, iModelId, [this](auto Changesets) {
		TArray<FChangesetInfo> copiedArray;
		for (const auto& info : Changesets)
		{
			copiedArray.Add({ info.Id, info.DisplayName, info.Description, info.Index });
		}
		this->OnGetiModelChangesetsComplete.Broadcast(true, { std::move(copiedArray) });
	});
}

void UiTwinWebServices::GetExports(FString iModelId, FString iChangesetId)
{
	FITwinServices::GetiModelExports(*CancelRequest, iModelId, iChangesetId, [this](auto Exports) {
		TArray<FExportInfo> copiedArray;
		for (const auto& info : Exports)
		{
			copiedArray.Add({ info.Id, info.DisplayName, info.Status, info.iModelId, info.ChangesetId, info.MeshUrl });
		}
		this->OnGetExportsComplete.Broadcast(true, { std::move(copiedArray) });
	});
}

