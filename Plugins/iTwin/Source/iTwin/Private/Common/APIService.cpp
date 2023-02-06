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

FAPIService::FRequestPtr FAPIService::SendPostRequest(const FString& URL, const FString& RequestContent, TArray<FAPIService::FHttpHeader> Headers, TFunction<void(TSharedPtr<FJsonObject>, const FString&)> Callback)
{
	// Reference code: https://dev.epicgames.com/community/learning/tutorials/ZdXD/call-rest-api-using-http-json-from-ue5-c

	FHttpModule& HttpModule = FHttpModule::Get();
	FRequestPtr Request = HttpModule.CreateRequest();

	// Preparing a POST request
	Request->SetVerb(TEXT("POST"));

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

FAPIService::FRequestPtr FAPIService::SendGetRequest(const FString& URL, const FString& RequestContent, TArray<FAPIService::FHttpHeader> Headers, TFunction<void(TSharedPtr<FJsonObject>, const FString&)> Callback)
{
	FHttpModule& HttpModule = FHttpModule::Get();
	FRequestPtr Request = HttpModule.CreateRequest();

	// Preparing a GET request
	Request->SetVerb(TEXT("GET"));

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
