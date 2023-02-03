/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "DataRequestManager.h"

#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Async/Async.h"
#include "Runtime/Online/HTTP/Public/HttpModule.h"
#include "Interfaces/IHttpResponse.h"

DEFINE_LOG_CATEGORY(LogHttpRequestClient);

FDataRequestManager::~FDataRequestManager()
{
	CancelAllHttpRequests();
}

void FDataRequestManager::SetBaseUrl(FString Url)
{
	auto LocalPath = FPaths::Combine(FPaths::LaunchDir(), FString("iModels"), FPaths::GetPathLeaf(Url));

	if (FPaths::DirectoryExists(LocalPath))
	{
		LocalUrl = "file:///" + LocalPath;
	}
	else
	{
		UrlParams = "";
		if (Url.StartsWith("http")) {
            FString Left, Right;
            if (Url.Split(TEXT("?"), &Left, &Right)) {
                BaseUrl = Left;
                UrlParams = Right;
                // UE_LOG(LogTemp, Warning, TEXT("Split base url %s from params %s"), *BaseUrl, *UrlParams);
                return;
            }
        }
		BaseUrl = Url;
	}
}

void FDataRequestManager::AddRequest(FString RelativeUrl, HttpRequestCallback Callback)
{
	auto Filename = RelativeUrl.Replace(TEXT("\\"), TEXT("/"));
	if (!Filename.EndsWith(".bin"))
	{
		Filename += ".bin";
	}

	// Calculate the full url taking into account the cache
	FString CacheFilePath;
	auto RequestUrl = FPaths::Combine(LocalUrl != "" ? LocalUrl : BaseUrl, Filename);
	if (RequestUrl.StartsWith("http"))
	{
		if (bUseCache)
		{
			CacheFilePath = FPaths::Combine(FPaths::Combine(FPaths::ProjectPersistentDownloadDir(), FPaths::GetPathLeaf(BaseUrl)), Filename);
			CacheFilePath = FPaths::ConvertRelativePathToFull(CacheFilePath);

			auto& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
			if (PlatformFile.FileExists(*CacheFilePath))
			{
				RequestUrl = PlatformFile.GetFilenameOnDisk(*CacheFilePath);
				CacheFilePath = "";
			}
			else
			{
				RequestUrl += ".gz";
			}
		}
		else
		{
			RequestUrl += ".gz";
		}
	}

	if (RequestUrl.StartsWith("http"))
	{
		RequestUrl = RequestUrl.Replace(TEXT(" "), TEXT("%20"));
        if (!UrlParams.IsEmpty()) {
            RequestUrl += "?";
            RequestUrl += UrlParams;
            UE_LOG(LogTemp, Error, TEXT("Requesting resource %s"), *RequestUrl);
        }
		AddHttpRequest(RequestUrl, [this, RelativeUrl, CacheFilePath, Callback](const uint8* Data, size_t Size, FString CompleteUrl) {
			Callback(Data, Size, RelativeUrl);
			if (CacheFilePath != "" && Data && Size > 0)
			{
				AsyncTask(ENamedThreads::AnyHiPriThreadNormalTask, [Data = TArray<uint8>(Data, Size), Path = CacheFilePath]() {
					FFileHelper::SaveArrayToFile(Data, *Path);
				});
			}
			});
	}
	else
	{
#if PLATFORM_WINDOWS
		RequestUrl = RequestUrl.Replace(TEXT("file:///"), TEXT("")).Replace(TEXT("/"), TEXT("\\"));
#endif
		RequestUrl = RequestUrl.Replace(TEXT("%20"), TEXT(" "));
		AsyncTask(ENamedThreads::AnyHiPriThreadNormalTask, [this, RelativeUrl, RequestUrl, Callback]() {
			TArray<uint8> FileData;
			if (FFileHelper::LoadFileToArray(FileData, *RequestUrl)) {
				Callback(FileData.GetData(), FileData.Num(), RelativeUrl);
			}
			else {
				UE_LOG(LogTemp, Error, TEXT("failed opening file %s"), *RequestUrl);
				Callback(nullptr, 0, RelativeUrl);
			}
			});
	}
}

void FDataRequestManager::CleanUpCache()
{
	// TBC
#if ((UE_BUILD_SHIPPING || UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG) && UE_GAME)
#else
#endif
}

void FDataRequestManager::AddHttpRequest(FString Url, HttpRequestCallback Callback)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetVerb("GET");
	HttpRequest->SetHeader("Content-Type", "application/octet-stream");
	HttpRequest->SetURL(Url);

	HttpRequest->OnProcessRequestComplete().BindLambda([this, Url, Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
		{
			RemovePendingHttpRequest(Request);

			if (bSuccess && Response) {
				auto Code = Response->GetResponseCode();

				if (Code == 0 || Code == 200) {
					auto Content = Response->GetContent();
					return Callback(Content.GetData(), Content.Num(), Url);
				}
				else {
					UE_LOG(LogHttpRequestClient, Error, TEXT("Failed to get the file: %s (code %d)"), *Response->GetURL(), Code);
				}
			}
			else
			{
				UE_LOG(LogHttpRequestClient, Error, TEXT("Failed to get the file: %s (unknown error)"), *Url);
			}
			return Callback(nullptr, 0, Url);
		});

	ActiveHttpRequests.Add(HttpRequest);

	HttpRequest->ProcessRequest();
}

void FDataRequestManager::RemovePendingHttpRequest(FHttpRequestPtr Request)
{
	for (int i = 0; i < ActiveHttpRequests.Num(); i++)
	{
		if (&ActiveHttpRequests[i].Get() == Request.Get())
		{
			ActiveHttpRequests.RemoveAt(i);
			return;
		}
	}
}

void FDataRequestManager::CancelAllHttpRequests()
{
	for (auto Request : ActiveHttpRequests)
	{
		Request->OnProcessRequestComplete().Unbind();
		Request->CancelRequest();
	}
}
