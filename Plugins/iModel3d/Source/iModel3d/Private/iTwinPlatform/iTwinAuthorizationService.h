/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <mutex>

#include "CoreMinimal.h"

#include "IHttpRouter.h"

#include "Common/SpinLock.h"
#include "Common/AutoCancelAsync.h"

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
	FAutoCancelAsync RefreshTickerHandle;

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

