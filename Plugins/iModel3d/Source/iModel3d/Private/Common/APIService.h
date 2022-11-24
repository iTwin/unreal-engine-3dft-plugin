/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "CoreMinimal.h"

struct FAPIService
{
public:
	static void SendPostRequest(const FString& URL, const FString& RequestContent, TFunction<void(TSharedPtr<FJsonObject>, const FString&)> Callback, const FString& Hostname);

	static void SendPostRequest(const FString& URL, const FString& RequestContent, TFunction<void(TSharedPtr<FJsonObject>, const FString&)> Callback);

	static void SendGetRequest(const FString& URL, const FString& RequestContent, const FString &AuthToken, TFunction<void(const FString&, const FString&)> Callback);
};

