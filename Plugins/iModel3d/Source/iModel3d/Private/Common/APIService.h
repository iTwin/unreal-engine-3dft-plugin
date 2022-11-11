#pragma once

#include "CoreMinimal.h"

struct FAPIService
{
public:
	static void SendPostRequest(const FString& URL, const FString& RequestContent, TFunction<void(TSharedPtr<FJsonObject>, const FString&)> Callback, const FString& Hostname);

	static void SendGetRequest(const FString& URL, const FString& RequestContent, const FString &AuthToken, TFunction<void(const FString&, const FString&)> Callback);
};

