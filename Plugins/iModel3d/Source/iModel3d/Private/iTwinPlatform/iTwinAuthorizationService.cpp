/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "iTwinAuthorizationService.h"

#include "Misc/App.h"
#include "Misc/MessageDialog.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "JsonUtilities.h"
#include "Misc/Base64.h"
#include "SHA256/SHA256.h"

#include "HttpPath.h"
#include "IHttpRouter.h"
#include "HttpServerHttpVersion.h"
#include "HttpServerModule.h"
#include "HttpServerResponse.h"
#include "HttpServerRequest.h"
#include "HttpResultCallback.h"

#include "Engine.h"

#include "Common/APIService.h"

struct FAuthorizationCredentials
{
	static constexpr auto Server = TEXT("https://ims.bentley.com");
	static constexpr auto AuthorizationEndpoint = TEXT("/connect/authorize");
	static constexpr auto TokenEndpoint = TEXT("/connect/token");
	static constexpr auto LocalhostPort = 24363;
	static constexpr auto RedirectUri = TEXT("http://localhost:24363/authorize");
	static constexpr auto iTwinAppId = TEXT("unreal-test");
	static constexpr auto Scope = TEXT("mesh-export:modify imodelaccess:read imodels:read itwins:read mesh-export:read offline_access");
};

namespace
{
FString GenerateRandomCharacters(uint32 AmountOfCharacters)
{
	const char* Values = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
	FString Result = "";
	for (uint32 i = 0; i < AmountOfCharacters; ++i)
	{
		Result.AppendChar(Values[FMath::RandRange(0, 61)]);
	}
	return Result;
}

FString LaunchWebBrowser(const FString& State, const FString &CodeVerifier)
{
	SHA256 Sha256;
	Sha256.update((const uint8_t*)TCHAR_TO_ANSI(*CodeVerifier), 128);

	auto Hash = FBase64::Encode(Sha256.digest(), 32);

	auto CodeChallenge = Hash.Replace(TEXT("+"), TEXT("-")).Replace(TEXT("/"), TEXT("_")).Replace(TEXT("="), TEXT(""));

	auto Scope = FString(FAuthorizationCredentials::Scope).Replace(TEXT(":"), TEXT("%3A")).Replace(TEXT(" "), TEXT("%20"));
	auto RedirectUri = FString(FAuthorizationCredentials::RedirectUri).Replace(TEXT(":"), TEXT("%3A")).Replace(TEXT("/"), TEXT("%2F"));

	auto Params = FString::Printf(
		TEXT("redirect_uri=%s&client_id=%s&response_type=code&state=%s&scope=%s&code_challenge=%s&code_challenge_method=S256"),
		*RedirectUri, FAuthorizationCredentials::iTwinAppId, *State, *Scope, *CodeChallenge);

	auto LaunchURL = FString::Printf(TEXT("%s%s?%s"), FAuthorizationCredentials::Server, FAuthorizationCredentials::AuthorizationEndpoint, *Params);

	FString Error;
	FPlatformProcess::LaunchURL(*LaunchURL, nullptr, &Error);
	if (!Error.IsEmpty())
	{
		return FString::Printf(TEXT("Could not launch web browser! %s"), *Error);
	}
	return "";
}

void AuthorizationTokenRequest(FString RequestContent, std::function<void(FString, FString, FString, FString)> Callback, std::function<void(FString)> ErrorCallback)
{
	auto URL = FString::Printf(TEXT("%s%s"), FAuthorizationCredentials::Server, FAuthorizationCredentials::TokenEndpoint);

	FAPIService::SendPostRequest(URL, RequestContent, {{ "Content-Type", "application/x-www-form-urlencoded" }}, [Callback, ErrorCallback](TSharedPtr<FJsonObject> Response, const FString& ErrorMessage)
	{
		if (!ErrorMessage.IsEmpty())
		{
			ErrorCallback(FString::Printf(TEXT("Error getting authorization token. %s"), *ErrorMessage));
		}
		else
		{
			FString ErrorCode;
			Response->TryGetStringField(TEXT("error"), ErrorCode);
			if (!ErrorCode.IsEmpty())
			{
				ErrorCallback(FString::Printf(TEXT("Error getting authorization token. %s"), *ErrorCode));
			}
			else
			{
				FString AuthToken;
				if (!Response->TryGetStringField(TEXT("access_token"), AuthToken) || Response->GetStringField(TEXT("token_type")) != "Bearer")
				{
					ErrorCallback(TEXT("Bad oauth response"));
				}
				else
				{
					auto ExpiresIn = Response->GetStringField(TEXT("expires_in"));
					auto RefreshToken = Response->GetStringField(TEXT("refresh_token"));
					auto IdToken = Response->GetStringField(TEXT("id_token"));

					Callback(AuthToken, ExpiresIn, RefreshToken, IdToken);
				}
			}
		}
	});
}
}

TUniquePtr<FITwinAuthorizationService> FITwinAuthorizationService::Singleton;

FITwinAuthorizationService& FITwinAuthorizationService::Get()
{
	if (nullptr == Singleton)
	{
		check(IsInGameThread());
		Singleton = MakeUnique<FITwinAuthorizationService>();
		Singleton->InitiateAuthorization();
	}
	check(Singleton);
	return *Singleton;
}

FString FITwinAuthorizationService::GetAuthToken()
{
	std::unique_lock<std::mutex> lock(Mutex);
	return AuthToken;
}

FString FITwinAuthorizationService::GetLastError()
{
	std::unique_lock<std::mutex> lock(Mutex);
	return LastError;
}

FTSTicker::FDelegateHandle FITwinAuthorizationService::GetAuthTokenAsync(std::function<void(FString AuthToken)> Callback)
{
	if (FString(FAuthorizationCredentials::iTwinAppId) == "")
	{
		UpdateError("The iTwin App ID is missing. Please refer to the plugin documentation.");
		Callback("");
		return {};
	}
	else
	{
		auto Token = GetAuthToken();
		if (!Token.IsEmpty())
		{
			Callback(Token);
			return {};
		}
		else
		{
			return FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this, Callback](float Delta) -> bool
			{
				auto Token = GetAuthToken();
				if (!Token.IsEmpty())
				{
					Callback(Token);
					return false;
				}
				else
				{
					return true; // Continue ticking
				}
			}), 0.200); // 200ms
		}
	}
}

void FITwinAuthorizationService::UpdateAuthToken(FString Token)
{
	UE_LOG(LogTemp, Display, TEXT("iTwin AuthToken renewed!"));
	std::unique_lock<std::mutex> lock(Mutex);
	AuthToken = Token;
	LastError = "";
}

void FITwinAuthorizationService::UpdateError(FString ErrorMessage)
{
	UE_LOG(LogTemp, Error, TEXT("Error: %s"), *ErrorMessage); // FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Error));
	std::unique_lock<std::mutex> lock(Mutex);
	AuthToken = "";
	LastError = ErrorMessage;
}


void FITwinAuthorizationService::InitiateAuthorization()
{
	FString State = GenerateRandomCharacters(10);
	FString CodeVerifier = GenerateRandomCharacters(128);

	FHttpServerModule& HttpServerModule = FHttpServerModule::Get();
	auto HttpRouter = HttpServerModule.GetHttpRouter(FAuthorizationCredentials::LocalhostPort);
	if (HttpRouter.IsValid())
	{
		auto AuthorizeRouteHandle = HttpRouter->BindRoute(FHttpPath(TEXT("/authorize")), EHttpServerRequestVerbs::VERB_GET, [this, State, HttpRouter, CodeVerifier](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
		{
			if (Request.QueryParams.Contains("code") && Request.QueryParams.Contains("state") && Request.QueryParams["state"] == State)
			{
				auto AuthorizationCode = Request.QueryParams["code"];
				OnComplete(FHttpServerResponse::Create(TEXT("<h1>Sign in was successful!</h1>You can close this browser window and return to the application."), TEXT("text/html")));
				Authorization.AuthorizationCode = AuthorizationCode;
				Authorization.CodeVerifier = CodeVerifier;
				FHttpServerModule::Get().StopAllListeners();
				GetAuthorizationToken();
			}
			else if (Request.QueryParams.Contains("error"))
			{
				auto Html = FString::Printf(TEXT("<h1>Error signin in!</h1><br/>%s<br/><br/>You can close this browser window and return to the application."), *Request.QueryParams["error_description"]);
				OnComplete(FHttpServerResponse::Create(*Html, TEXT("text/html")));

				FHttpServerModule::Get().StopAllListeners();
				UpdateError(Request.QueryParams["error_description"]);
			}
			else
			{
				OnComplete(FHttpServerResponse::Create(TEXT(""), TEXT("text/html")));
				FHttpServerModule::Get().StopAllListeners();
				return true;
			}
			FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this, HttpRouter](float Delta) -> bool
			{
				return false; // One tick
			}), 0.0001f);
			return true;
		});

		if (!AuthorizeRouteHandle)
		{
			UpdateError(FString::Printf(TEXT("Could not bind '/authorize' route on server on port = %d. This binding may already be in use!"), FAuthorizationCredentials::LocalhostPort));
		}
		else
		{
			HttpServerModule.StartAllListeners();
			UE_LOG(LogTemp, Log, TEXT("Web server started on port = %d"), FAuthorizationCredentials::LocalhostPort);
			auto Error = LaunchWebBrowser(State, CodeVerifier);
			if (!Error.IsEmpty())
			{
				HttpServerModule.StartAllListeners();
				UpdateError(Error);
			}
		}
	}
	else
	{
		UpdateError(FString::Printf(TEXT("Could not start web server on port = %d. This post may already be in use!"), FAuthorizationCredentials::LocalhostPort));
	}
}

void FITwinAuthorizationService::GetAuthorizationToken()
{
	FString RequestContent = 
		FString::Printf(TEXT("grant_type=authorization_code&client_id=%s&redirect_uri=%s&code=%s&code_verifier=%s&scope=%s"),
			FAuthorizationCredentials::iTwinAppId, FAuthorizationCredentials::RedirectUri, *Authorization.AuthorizationCode, *Authorization.CodeVerifier, FAuthorizationCredentials::Scope);

	AuthorizationTokenRequest(RequestContent, [this](FString NewAuthToken, FString ExpiresIn, FString RefreshToken, FString IdToken) {
		Authorization.RefreshToken = RefreshToken;
		Authorization.ExpiresIn = FCString::Atoi(*ExpiresIn);
		Authorization.IdToken = IdToken;
		UpdateAuthToken(NewAuthToken);
		DelayRefreshAuthorizationToken();
		}, [this](FString Error) { UpdateError(Error); });
}

void FITwinAuthorizationService::DelayRefreshAuthorizationToken()
{
	RefreshTickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this](float Delta) -> bool
	{
		FString RequestContent =
			FString::Printf(TEXT("grant_type=refresh_token&client_id=%s&redirect_uri=%s&refresh_token=%s&code=%s&code_verifier=%s&scope=%s"),
				FAuthorizationCredentials::iTwinAppId, FAuthorizationCredentials::RedirectUri, *Authorization.RefreshToken, *Authorization.AuthorizationCode, *Authorization.CodeVerifier, FAuthorizationCredentials::Scope);

		AuthorizationTokenRequest(RequestContent, [this](FString NewAuthToken, FString ExpiresIn, FString RefreshToken, FString IdToken) {
			Authorization.RefreshToken = RefreshToken;
			Authorization.ExpiresIn = FCString::Atoi(*ExpiresIn);
			Authorization.IdToken = IdToken;
			UpdateAuthToken(NewAuthToken);
			DelayRefreshAuthorizationToken();
		}, [this](FString Error) { UpdateError(Error); });

		return false; // One tick
	}), Authorization.ExpiresIn);
}
