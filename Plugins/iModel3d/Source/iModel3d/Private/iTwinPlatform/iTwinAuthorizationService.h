#pragma once

#include <mutex>

#include "CoreMinimal.h"

#include "IHttpRouter.h"

#include "Common/SpinLock.h"

class UITwinAuthorizationService
{
	using NewTokenCallback = std::function<void(FString AuthToken, FString Error)>;
public:
	static UITwinAuthorizationService& UITwinAuthorizationService::Get();

	FString GetAuthToken();

	FString GetLastError();

	FTSTicker::FDelegateHandle GetAuthTokenAsync(std::function<void(FString AuthToken)> Callback);

private:
	void InitiateAuthorization();

	void GetAuthorizationToken();

	void DelayRefreshAuthorizationToken();

	void UpdateAuthToken(FString Token);

	void UpdateError(FString ErrorMessage);

	static TUniquePtr<UITwinAuthorizationService> Singleton;

	FHttpRouteHandle AuthorizeRouteHandle;

	std::mutex Mutex;
	FString LastError;
	FString AuthToken;

	struct {
		FString AuthorizationCode;
		FString CodeVerifier;
		FString RefreshToken;
		FString IdToken;
		int ExpiresIn;
	} Authorization;
};

