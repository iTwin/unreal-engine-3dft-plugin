#pragma once

#include "CoreMinimal.h"

#include "IHttpRouter.h"

class UITwinAuthorizationService
{
public:
	void InitiateAuthorization();

private:
	void GetAuthorizationToken(FString AuthorizationCode, FString CodeVerifier);

	void DelayRefreshAuthorizationToken(FString RefreshToken, FString AuthorizationCode, FString CodeVerifier, int DelaySeconds);

	FHttpRouteHandle AuthorizeRouteHandle;
};

