#pragma once

#include "CoreMinimal.h"

#include "IHttpRouter.h"

class UITwinAuthorizationService
{
	using NewTokenCallback = std::function<void(FString AuthToken, FString Error)>;
public:
	void InitiateAuthorization(NewTokenCallback Callback);

private:
	void GetAuthorizationToken(FString AuthorizationCode, FString CodeVerifier, NewTokenCallback Callback);

	void DelayRefreshAuthorizationToken(FString RefreshToken, FString AuthorizationCode, FString CodeVerifier, int DelaySeconds, NewTokenCallback Callback);

	FHttpRouteHandle AuthorizeRouteHandle;
};

