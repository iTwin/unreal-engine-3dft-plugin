#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "iTwinAuthenticationService.h"

#include "iTwinProjectsService.generated.h"

USTRUCT(BlueprintType)
struct FProject
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(BlueprintReadOnly)
		FString id;

	UPROPERTY(BlueprintReadOnly)
		FString displayName;

	UPROPERTY(BlueprintReadOnly)
		FString projectNumber;
};

USTRUCT(BlueprintType)
struct FProjectList
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(BlueprintReadOnly)
		TArray<FProject> projects;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOniTwinProjectList, FProjectList, ProjectList);

UCLASS(BlueprintType)
class IMODEL3D_API UiTwinProjectsService : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
		FOniTwinProjectList ProjectListReceived;

	UFUNCTION(BlueprintCallable)
		void GetProjects(const FAuthentication& Authentication);
};

