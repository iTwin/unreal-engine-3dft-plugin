/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "CoreMinimal.h"

#include "HttpModule.h"

struct FAPIService
{
public:
	struct FHttpHeader
	{
		FString Title;
		FString Content;
	};

	using FRequestPtr = TSharedRef<IHttpRequest, ESPMode::ThreadSafe>;
public:
	static FRequestPtr SendPostRequest(const FString& URL, const FString& RequestContent, TFunction<void(TSharedPtr<FJsonObject>, const FString&)> Callback);

	static FRequestPtr SendGetRequest(const FString& URL, const FString& RequestContent, const FString& AuthToken, TFunction<void(TSharedPtr<FJsonObject>, const FString&)> Callback, FString ApiVersion = "v1", TArray<FAPIService::FHttpHeader> Headers = {});
};

