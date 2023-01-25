#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "iTwinWebServices_Info.h"
#include "iTwinWebServices.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGetiTwinsComplete, bool, bSuccess, FiTwinInfos, iTwins);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGetiTwiniModelsComplete, bool, bSuccess, FiModelInfos, iModels);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGetiModelChangesetsComplete, bool, bSuccess, FChangesetInfos, Changesets);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGetExportsComplete, bool, bSuccess, FExportInfos, Exports);

struct FCancelRequest;

UCLASS(BlueprintType)
class UiTwinWebServices : public UObject
{
    GENERATED_BODY()

public:
    UiTwinWebServices();

    UFUNCTION(BlueprintCallable, Category = "iTwin Web Services")
        void GetiTwins();

    UFUNCTION(BlueprintCallable, Category = "iTwin Web Services")
        void GetiTwiniModels(FString iTwinId);

    UFUNCTION(BlueprintCallable, Category = "iTwin Web Services")
        void GetiModelChangesets(FString iModelId);

    UFUNCTION(BlueprintCallable, Category = "iTwin Web Services")
        void GetExports(FString iModelId, FString iChangesetId);

    UPROPERTY(BlueprintAssignable, Category = "iTwin Web Services")
        FOnGetiTwinsComplete OnGetiTwinsComplete;

    UPROPERTY(BlueprintAssignable, Category = "iTwin Web Services")
        FOnGetiTwiniModelsComplete OnGetiTwiniModelsComplete;

    UPROPERTY(BlueprintAssignable, Category = "iTwin Web Services")
        FOnGetiModelChangesetsComplete OnGetiModelChangesetsComplete;

    UPROPERTY(BlueprintAssignable, Category = "iTwin Web Services")
        FOnGetExportsComplete OnGetExportsComplete;


private:
    std::shared_ptr<FCancelRequest> CancelRequest;
};
