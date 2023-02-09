/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "iTwinWebServices/iTwinWebServices.h"
#include "iTwinPlatform/iTwinServices.h"

UiTwinWebServices::UiTwinWebServices()
{
	CancelRequest = std::make_shared<FCancelRequest>();
}

void UiTwinWebServices::CheckAuthorization()
{
	FITwinServices::CheckAuthorization(*CancelRequest, [this](bool bSuccess, FString Error) {
		this->OnAuthorizationChecked.Broadcast( bSuccess, Error );
	});
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

void UiTwinWebServices::GetExportInfo(FString ExportId)
{
	FITwinServices::GetExportInfo(*CancelRequest, ExportId, [this](auto info) {
		this->OnGetExportInfoComplete.Broadcast(true, { info.Id, info.DisplayName, info.Status, info.iModelId, info.ChangesetId, info.MeshUrl });
	});
}

void UiTwinWebServices::StartExport(FString iModelId, FString iChangesetId)
{
	FITwinServices::StartExport(*CancelRequest, iModelId, iChangesetId, [this](FString ExportId) {
		this->OnStartExportComplete.Broadcast(true, ExportId);
	});
}

void UiTwinWebServices::GetAllSavedViews(FString iTwinId, FString iModelId)
{
	FITwinServices::GetAllSavedViews(*CancelRequest, iTwinId, iModelId, [this](auto SavedViews) {
		TArray<FSavedViewInfo> copiedArray;
		for (const auto& info : SavedViews)
		{
			copiedArray.Add({ info.Id, info.DisplayName, info.bShared });
		}
		this->OnGetSavedViewsComplete.Broadcast(true, { std::move(copiedArray) });
	});
}

void UiTwinWebServices::GetSavedView(FString SavedViewId)
{
	FITwinServices::GetSavedView(*CancelRequest, SavedViewId, [this](auto SavedView) {
		this->OnGetSavedViewComplete.Broadcast(true, { SavedView.Origin, SavedView.Extents, SavedView.Angles });
	});
}