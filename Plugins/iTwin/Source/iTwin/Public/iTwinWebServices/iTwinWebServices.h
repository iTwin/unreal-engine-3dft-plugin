/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "iTwinWebServices_Info.h"
#include "iTwinWebServices.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAuthorizationChecked, bool, bSuccess, FString, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGetiTwinsComplete, bool, bSuccess, FiTwinInfos, iTwins);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGetiTwiniModelsComplete, bool, bSuccess, FiModelInfos, iModels);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGetiModelChangesetsComplete, bool, bSuccess, FChangesetInfos, Changesets);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGetExportsComplete, bool, bSuccess, FExportInfos, Exports);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGetExportInfoComplete, bool, bSuccess, FExportInfo, Export);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStartExportComplete, bool, bSuccess, FString, ExportId);

struct FCancelRequest;

UCLASS(BlueprintType)
class UiTwinWebServices : public UObject
{
    GENERATED_BODY()

public:
    UiTwinWebServices();

    UFUNCTION(BlueprintCallable, Category = "iTwin Web Services")
        void CheckAuthorization();

    UFUNCTION(BlueprintCallable, Category = "iTwin Web Services")
        void GetiTwins();

    UFUNCTION(BlueprintCallable, Category = "iTwin Web Services")
        void GetiTwiniModels(FString iTwinId);

    UFUNCTION(BlueprintCallable, Category = "iTwin Web Services")
        void GetiModelChangesets(FString iModelId);

    UFUNCTION(BlueprintCallable, Category = "iTwin Web Services")
        void GetExports(FString iModelId, FString iChangesetId);

    UFUNCTION(BlueprintCallable, Category = "iTwin Web Services")
        void GetExportInfo(FString ExportId);

    UFUNCTION(BlueprintCallable, Category = "iTwin Web Services")
        void StartExport(FString iModelId, FString iChangesetId);

    UPROPERTY(BlueprintAssignable, Category = "iTwin Web Services")
        FOnAuthorizationChecked OnAuthorizationChecked;

    UPROPERTY(BlueprintAssignable, Category = "iTwin Web Services")
        FOnGetiTwinsComplete OnGetiTwinsComplete;

    UPROPERTY(BlueprintAssignable, Category = "iTwin Web Services")
        FOnGetiTwiniModelsComplete OnGetiTwiniModelsComplete;

    UPROPERTY(BlueprintAssignable, Category = "iTwin Web Services")
        FOnGetiModelChangesetsComplete OnGetiModelChangesetsComplete;

    UPROPERTY(BlueprintAssignable, Category = "iTwin Web Services")
        FOnGetExportsComplete OnGetExportsComplete;

    UPROPERTY(BlueprintAssignable, Category = "iTwin Web Services")
        FOnGetExportInfoComplete OnGetExportInfoComplete;

    UPROPERTY(BlueprintAssignable, Category = "iTwin Web Services")
        FOnStartExportComplete OnStartExportComplete;

private:
    std::shared_ptr<FCancelRequest> CancelRequest;
};
