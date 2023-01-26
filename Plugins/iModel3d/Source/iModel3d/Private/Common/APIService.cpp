/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "APIService.h"

#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "JsonUtilities.h"

namespace
{
void ParseResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, const TFunction<void(TSharedPtr<FJsonObject>, const FString&)> &Callback)
{
	if (bConnectedSuccessfully)
	{
		const FString& ResponseString = Response->GetContentAsString();

		TSharedPtr<FJsonObject> JsonRoot;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);
		if (!FJsonSerializer::Deserialize(Reader, JsonRoot) || !JsonRoot.IsValid())
		{
			// In some cases, Response string will contain an error message
			auto ErrorMessage = ResponseString.IsEmpty() ? TEXT("Problem parsing response") : ResponseString;
			TSharedPtr<FJsonObject> DummyJson;
			Callback(DummyJson, ErrorMessage);
		}
		else
		{
			Callback(JsonRoot, FString());
		}
	}
	else
	{
		TSharedPtr<FJsonObject> DummyJson;
		Callback(DummyJson, EHttpRequestStatus::ToString(Request->GetStatus()));
	}
}
}

FAPIService::FRequestPtr FAPIService::SendPostRequest(const FString& URL, const FString& RequestContent, TFunction<void(TSharedPtr<FJsonObject>, const FString&)> Callback)
{
	// Reference code: https://dev.epicgames.com/community/learning/tutorials/ZdXD/call-rest-api-using-http-json-from-ue5-c

	FHttpModule& HttpModule = FHttpModule::Get();
	FRequestPtr Request = HttpModule.CreateRequest();

	// Preparing a POST request
	Request->SetVerb(TEXT("POST"));

	// We'll need to tell the server what type of content to expect in the POST data
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/x-www-form-urlencoded"));

	Request->SetContentAsString(RequestContent);

	// Set the http URL
	Request->SetURL(URL);

	// Set the callback, which will execute when the HTTP call is complete
	Request->OnProcessRequestComplete().BindLambda([Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
	{
		ParseResponse(Request, Response, bConnectedSuccessfully, Callback);
	});

	// Submit the request
	Request->ProcessRequest();
	return Request;
}

FAPIService::FRequestPtr FAPIService::SendGetRequest(const FString& URL, const FString& RequestContent, const FString& AuthToken, TFunction<void(TSharedPtr<FJsonObject>, const FString&)> Callback, FString ApiVersion, TArray<FAPIService::FHttpHeader> Headers)
{
	FHttpModule& HttpModule = FHttpModule::Get();
	FRequestPtr Request = HttpModule.CreateRequest();

	// Preparing a GET request
	Request->SetVerb(TEXT("GET"));

	// Set authorization header
	Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *AuthToken));
	
	auto Accept = FString::Printf(TEXT("application/vnd.bentley.itwin-platform.%s+json"), *ApiVersion);
	Request->SetHeader(TEXT("Accept"), Accept);

	for (const auto& Header : Headers)
	{
		Request->SetHeader(Header.Title, Header.Content);
	}
	
	Request->SetContentAsString(RequestContent);

	// Set the http URL
	Request->SetURL(URL);

	// Set the callback, which will execute when the HTTP call is complete
	Request->OnProcessRequestComplete().BindLambda([Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
	{
		ParseResponse(Request, Response, bConnectedSuccessfully, Callback);
	});

	// Submit the request
	Request->ProcessRequest();
	return Request;
}
