/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "iTwinAuthenticationService.generated.h"

class FJsonObject;

UENUM(BlueprintType)
enum class EAuthenticationStatus : uint8
{
	AuthenticationError,
	RequestedToken,
	Authenticated,
};

USTRUCT(BlueprintType)
struct FAuthentication
{
	GENERATED_BODY()

	FString DeviceCode;

	FString AuthToken;

	float AuthPollInterval = 3.f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAuthenticationChange, FString, AuthToken, EAuthenticationStatus, State);

UCLASS(BlueprintType)
class IMODEL3D_API UiTwinAuthenticationService : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
		FOnAuthenticationChange AuthenticationChange;

	UFUNCTION(BlueprintCallable)
		void InitiateAuthentication();

private:
	void HandleOauthResponse(TSharedPtr<FJsonObject> Response, const FString& ErrorMessage);

	void HandleTokenResponse(TSharedPtr<FJsonObject> Response, const FString& ErrorMessage, FAuthentication Authentication);

	void WaitAndRequestOauthToken(FAuthentication Authentication);

	void AuthenticationError(const FString& ErrorMessage);
};

