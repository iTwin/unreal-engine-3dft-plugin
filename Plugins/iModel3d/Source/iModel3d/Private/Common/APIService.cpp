/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "APIService.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "JsonUtilities.h"

void FAPIService::SendPostRequest(const FString& URL, const FString& RequestContent, TFunction<void(TSharedPtr<FJsonObject>, const FString&)> Callback, const FString& Hostname)
{
	FAPIService::SendPostRequest(FString(TEXT("https://")) + Hostname + URL, RequestContent, Callback);
}

void FAPIService::SendPostRequest(const FString& URL, const FString& RequestContent, TFunction<void(TSharedPtr<FJsonObject>, const FString&)> Callback)
{
	// Reference code: https://dev.epicgames.com/community/learning/tutorials/ZdXD/call-rest-api-using-http-json-from-ue5-c

	FHttpModule& HttpModule = FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule.CreateRequest();

	// Preparing a POST request
	Request->SetVerb(TEXT("POST"));

	// We'll need to tell the server what type of content to expect in the POST data
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/x-www-form-urlencoded"));

	Request->SetContentAsString(RequestContent);

	// Set the http URL
	Request->SetURL(URL);

	// Set the callback, which will execute when the HTTP call is complete
	Request->OnProcessRequestComplete().BindLambda(
		[Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
		{
			bool bHandled = false;
			FString ErrorMessage;

			if (bConnectedSuccessfully)
			{
				// We should have a JSON response - attempt to process it.
				const FString& ResponseString = Response->GetContentAsString();

				TSharedPtr<FJsonObject> JsonRoot;
				TSharedRef<TJsonReader<> > Reader = TJsonReaderFactory<>::Create(ResponseString);
				if (!FJsonSerializer::Deserialize(Reader, JsonRoot) || !JsonRoot.IsValid())
				{
					// In some cases, Response string will contain an error message
					ErrorMessage = ResponseString.IsEmpty() ? TEXT("Problem parsing response") : ResponseString;
				}
				else
				{
					Callback(JsonRoot, ErrorMessage);
					bHandled = true;
				}
			}
			else
			{
				ErrorMessage = EHttpRequestStatus::ToString(Request->GetStatus());
			}

			if (!bHandled)
			{
				// Execute callback with error string
				TSharedPtr<FJsonObject> DummyJson;
				Callback(DummyJson, ErrorMessage);
			}
		});

	// Submit the request
	Request->ProcessRequest();
}

void FAPIService::SendGetRequest(const FString& URL, const FString& RequestContent, const FString& AuthToken, TFunction<void(const FString&, const FString&)> Callback)
{
	// Reference code: https://dev.epicgames.com/community/learning/tutorials/ZdXD/call-rest-api-using-http-json-from-ue5-c

	FHttpModule& HttpModule = FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule.CreateRequest();

	// Preparing a GET request
	Request->SetVerb(TEXT("GET"));

	// Set authorization header
	Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *AuthToken));

	Request->SetContentAsString(RequestContent);

	// Set the http URL
	Request->SetURL(URL);

	// Set the callback, which will execute when the HTTP call is complete
	Request->OnProcessRequestComplete().BindLambda(
		[Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
		{
			bool bHandled = false;
			FString ErrorMessage;

			if (bConnectedSuccessfully)
			{
				// We should have a JSON response - attempt to process it.
				const FString& ResponseString = Response->GetContentAsString();

				Callback(ResponseString, ErrorMessage);
				bHandled = true;

				UE_LOG(LogTemp, Warning, TEXT("%s"), *ResponseString);
			}
			else
			{
				ErrorMessage = EHttpRequestStatus::ToString(Request->GetStatus());
			}

			if (!bHandled)
			{
				// Execute callback with error string
				TSharedPtr<FJsonObject> DummyJson;
				Callback(FString(), ErrorMessage);
			}
		});

	// Submit the request
	Request->ProcessRequest();
}
