#pragma once

#include "CoreMinimal.h"

#include "Interfaces/IHttpRequest.h"

#include <functional>

class FDataRequestManager
{
public:
	~FDataRequestManager();

	typedef std::function<void(const uint8* Data, size_t Size, FString Url)> HttpRequestCallback;
	void AddRequest(FString RelativeUrl, HttpRequestCallback Callback);

	void SetBaseUrl(FString Url);
	void SetUseCache(bool UseCache) { bUseCache = UseCache; };

private:
	void CleanUpCache();

	void RemovePendingHttpRequest(FHttpRequestPtr Request);

	void AddHttpRequest(FString Url, HttpRequestCallback Callback);

	void CancelAllHttpRequests();


	bool bUseCache = true;
	FString BaseUrl;
    FString UrlParams;
	FString LocalUrl;
	TArray<TSharedRef<IHttpRequest, ESPMode::ThreadSafe>> ActiveHttpRequests;
};

DECLARE_LOG_CATEGORY_EXTERN(LogHttpRequestClient, Error, All);
