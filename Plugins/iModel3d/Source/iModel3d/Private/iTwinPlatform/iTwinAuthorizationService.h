#pragma once

#include "CoreMinimal.h"

#include "IHttpRouter.h"

class FJsonObject;

enum class EAuthorizationStatus : uint8
{
	Uninitialized,
	AuthorizationError,
	RequestedToken,
	Authenticated,
};

struct FAuthorization
{
	FString DeviceCode;
	FString AuthToken;
	float AuthPollInterval = 3.f;
};

// DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAuthorizationChange, FAuthorization, Authorization, EAuthorizationStatus, State);

class UITwinAuthorizationService
{
public:
	// FOnAuthorizationChange AuthorizationChange;

	void InitiateAuthorization();
	// void CancelAllRequests();

private:
	/*
	void HandleOauthResponse(TSharedPtr<FJsonObject> Response, const FString& ErrorMessage);

	void HandleTokenResponse(TSharedPtr<FJsonObject> Response, const FString& ErrorMessage, FAuthorization Authorization);

	void WaitAndRequestOauthToken(FAuthorization Authorization);

	void AuthorizationError(const FString& ErrorMessage);

	FTSTicker::FDelegateHandle TickerHandle;*/

	void GetAuthorizationToken(FString AuthorizationCode, FString CodeVerifier);

	FHttpRouteHandle AuthorizeRouteHandle;
};

