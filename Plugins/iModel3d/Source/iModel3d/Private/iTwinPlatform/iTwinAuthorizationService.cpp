#include "iTwinAuthorizationService.h"

#include "Containers/Ticker.h"
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
	static constexpr auto ClientId = TEXT("unreal-test");
	static constexpr auto Scope = TEXT("openid imodels:read mesh-export:modify mesh-export:read offline_access");
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

void LaunchWebBrowser(const FString& State, const FString &CodeVerifier)
{
	SHA256 Sha256;
	Sha256.update((const uint8_t*)TCHAR_TO_ANSI(*CodeVerifier), 128);

	auto Hash = FBase64::Encode(Sha256.digest(), 32);

	auto CodeChallenge = Hash.Replace(TEXT("+"), TEXT("-")).Replace(TEXT("/"), TEXT("_")).Replace(TEXT("="), TEXT(""));

	auto Scope = FString(FAuthorizationCredentials::Scope).Replace(TEXT(":"), TEXT("%3A")).Replace(TEXT(" "), TEXT("%20"));
	auto RedirectUri = FString(FAuthorizationCredentials::RedirectUri).Replace(TEXT(":"), TEXT("%3A")).Replace(TEXT("/"), TEXT("%2F"));

	auto Params = FString::Printf(
		TEXT("redirect_uri=%s&client_id=%s&response_type=code&state=%s&scope=%s&code_challenge=%s&code_challenge_method=S256"),
		*RedirectUri, FAuthorizationCredentials::ClientId, *State, *Scope, *CodeChallenge);

	auto LaunchURL = FString::Printf(TEXT("%s%s?%s"), FAuthorizationCredentials::Server, FAuthorizationCredentials::AuthorizationEndpoint, *Params);

	FString Error;
	FPlatformProcess::LaunchURL(*LaunchURL, nullptr, &Error);
	if (Error.Len() != 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not launch web browser!")); // FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Error));
	}
}


	/*
	function AuthorizationTokenRequest(json_data, callback, error_callback)
{
  var post_data = querystring.stringify(json_data);
  var post_options =
  {
	  host: 'ims.bentley.com',
	  port: '443 ',
	  path: '/connect/token',
	  method: 'POST',
	  headers: {
		  'Content-Type': 'application/x-www-form-urlencoded',
		  'Content-Length': Buffer.byteLength(post_data)
	  }
  };

  // Set up the request
  var post_req = https.request(post_options, function(res) {
	  res.setEncoding('utf8');
	  res.on('data', function (chunk) {
		const response = JSON.parse(chunk);
		if (response.access_token && response.token_type == "Bearer")
		{
		  callback(response.access_token, response.expires_in, response.refresh_token, response.id_token)
		}
		else
		{
		  error_callback(response.error, response.error_description)
		}
	  });
  });

  post_req.write(post_data);
  post_req.end();
}

*/

}

void UITwinAuthorizationService::InitiateAuthorization()
{
	FString State = GenerateRandomCharacters(10);
	FString CodeVerifier = GenerateRandomCharacters(128);

	FHttpServerModule& HttpServerModule = FHttpServerModule::Get();
	auto HttpRouter = HttpServerModule.GetHttpRouter(FAuthorizationCredentials::LocalhostPort);
	if (HttpRouter.IsValid())
	{
		AuthorizeRouteHandle = HttpRouter->BindRoute(FHttpPath(TEXT("/authorize")), EHttpServerRequestVerbs::VERB_GET, [this, State, HttpRouter, CodeVerifier](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) {
			UE_LOG(LogTemp, Log, TEXT("Get requested"));
			if (Request.QueryParams.Contains("code") && Request.QueryParams.Contains("state") && Request.QueryParams["state"] == State)
			{
				auto AuthorizationCode = Request.QueryParams["code"];
				OnComplete(FHttpServerResponse::Create(TEXT("<h1>Sign in was successful!</h1>You can close this browser window and return to the application."), TEXT("text/html")));
				GetAuthorizationToken(AuthorizationCode, CodeVerifier);
			}
			else if (Request.QueryParams.Contains("error"))
			{
				auto Html = FString::Printf(TEXT("<h1>Error signin in!</h1><br/>%s<br/><br/>You can close this browser window and return to the application."), *Request.QueryParams["error_description"]);
				OnComplete(FHttpServerResponse::Create(*Html, TEXT("text/html")));
			}
			else
			{
				OnComplete(FHttpServerResponse::Create(TEXT(""), TEXT("text/html")));
				return true;
			}
			FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this, HttpRouter](float Delta) -> bool
			{
				if (HttpRouter.IsValid() && this->AuthorizeRouteHandle)
				{
					HttpRouter->UnbindRoute(this->AuthorizeRouteHandle);
				}
				return false; // One tick
			}), 0.0001f);
			return true;
		});

		if (!AuthorizeRouteHandle)
		{
			UE_LOG(LogTemp, Error, TEXT("Could not bind '/authorize' route on server on port = %d. This binding may already be in use!"), FAuthorizationCredentials::LocalhostPort);
			return;
		}

		HttpServerModule.StartAllListeners();
		UE_LOG(LogTemp, Log, TEXT("Web server started on port = %d"), FAuthorizationCredentials::LocalhostPort);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Could not start web server on port = %d"), FAuthorizationCredentials::LocalhostPort);
		return;
	}

	LaunchWebBrowser(State, CodeVerifier);
}

void UITwinAuthorizationService::GetAuthorizationToken(FString AuthorizationCode, FString CodeVerifier)
{
	FString RequestContent = FString::Printf(
		TEXT("grant_type=authorization_code&client_id=%s&redirect_uri=%s&code=%s&code_verifier=%s&scope=%s"),
		FAuthorizationCredentials::ClientId, FAuthorizationCredentials::RedirectUri, *AuthorizationCode, *CodeVerifier, FAuthorizationCredentials::Scope);

	auto URL = FString::Printf(TEXT("%s%s"), FAuthorizationCredentials::Server, FAuthorizationCredentials::TokenEndpoint);

	FAPIService::SendPostRequest(URL, RequestContent, [this](TSharedPtr<FJsonObject> Response, const FString& ErrorMessage)
	{
		// HandleOauthResponse(Response, ErrorMessage);

			if (!ErrorMessage.IsEmpty())
			{
				UE_LOG(LogTemp, Error, TEXT("Error getting authorization token. %s"), *ErrorMessage);
			}
			else
			{
				FString ErrorCode;
				Response->TryGetStringField(TEXT("error"), ErrorCode);
				if (!ErrorCode.IsEmpty())
				{
					UE_LOG(LogTemp, Error, TEXT("Error getting authorization token. %s"), *ErrorCode);
				}
				else
				{
					FString AuthToken;
					if (!Response->TryGetStringField(TEXT("access_token"), AuthToken))
					{
						UE_LOG(LogTemp, Error, TEXT("Bad oauth response"));
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("Success: %s"), *AuthToken);
					}
				}
			}
	});

	//const json_data = { grant_type: 'authorization_code', client_id : 'unreal-test', redirect_uri : redirect_uri, code : auth_code, code_verifier : code_verifier, scope : scope };
	//AuthorizationTokenRequest(json_data, callback, error_callback)
}


/*
void UITwinAuthorizationService::CancelAllRequests()
{
	// Cancel the token request
	if (TickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
		TickerHandle.Reset();
	}

	AuthorizationChange.Broadcast({}, EAuthorizationStatus::Uninitialized);
}

void UITwinAuthorizationService::AuthorizationError(const FString& ErrorMessage)
{
	UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);

	AuthorizationChange.Broadcast({}, EAuthorizationStatus::AuthorizationError);
}

void UITwinAuthorizationService::HandleOauthResponse(TSharedPtr<FJsonObject> Response, const FString& ErrorMessage)
{
	FString ErrorCode;

	// Firstly, handle possible errors
	if (!ErrorMessage.IsEmpty())
	{
		ErrorCode = ErrorMessage;
	}
	else
	{
		// We have a json, check if there's an error inside
		Response->TryGetStringField(TEXT("error"), ErrorCode);
		FString ErrorDescription;
		if (Response->TryGetStringField(TEXT("error_description"), ErrorCode))
		{
			ErrorCode = FString::Printf(TEXT("%s: %s"), *ErrorCode, *ErrorDescription);
		}
	}

	if (!ErrorCode.IsEmpty())
	{
		AuthorizationError(ErrorCode);
		return;
	}

	FAuthorization Authorization;

	// No error at this point, handle the response now.
	// Retrieve the URL for verification and the device code.
	FString LaunchURL;
	if (!Response->TryGetStringField(TEXT("verification_uri_complete"), LaunchURL) ||
		!Response->TryGetStringField(TEXT("device_code"), Authorization.DeviceCode))
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid oauth response"));
		return;
	}

	// Read the poll interval
	double PollInterval = 3.0;
	Response->TryGetNumberField(TEXT("interval"), PollInterval);
	Authorization.AuthPollInterval = (float)PollInterval;

	FString Error;
	FPlatformProcess::LaunchURL(*LaunchURL, nullptr, &Error);
	if (Error.Len() != 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Error));
		return;
	}

	WaitAndRequestOauthToken(Authorization);
}

void UITwinAuthorizationService::WaitAndRequestOauthToken(FAuthorization Authorization)
{
	AuthorizationChange.Broadcast(Authorization, EAuthorizationStatus::RequestedToken);

	TickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda(
		[this, Authorization](float Delta) -> bool
		{
			// Send the token request
			FString RequestContent = FString::Printf(
				TEXT("client_id=%s&device_code=%s&grant_type=%s"),
				FAuthorizationCredentials::ClientId, *Authorization.DeviceCode, TEXT("urn:ietf:params:oauth:grant-type:device_code"));
			FAPIService::SendPostRequest(FAuthorizationCredentials::TokenEndpoint, RequestContent, [this, Authorization](TSharedPtr<FJsonObject> Response, const FString& ErrorMessage)
				{
					HandleTokenResponse(Response, ErrorMessage, Authorization);
				}, FAuthorizationCredentials::Hostname);
			// Delete and stop the ticker (reset delegate and 'return false')
			// We have a valid http request now, and its handler may initiate a new timer, but at the monent we
			// should remove the current timer
			TickerHandle.Reset();
			return false;
		}), Authorization.AuthPollInterval);
}

void UITwinAuthorizationService::HandleTokenResponse(TSharedPtr<FJsonObject> Response, const FString& ErrorMessage, FAuthorization Authorization)
{
	if (!ErrorMessage.IsEmpty())
	{
		AuthorizationError(ErrorMessage);
		return;
	}

	FString ErrorCode;
	Response->TryGetStringField(TEXT("error"), ErrorCode);
	if (ErrorCode == TEXT("authorization_pending"))
	{
		// User still have to authenticate in browser, repeat the request
		WaitAndRequestOauthToken(Authorization);
		return;
	}

	// Other errors:
	// - "slow_down" = polling too frequently
	// - "access_denied" = user declined the authorization
	if (!ErrorCode.IsEmpty())
	{
		AuthorizationError(ErrorCode);
		return;
	}

	if (!Response->TryGetStringField(TEXT("access_token"), Authorization.AuthToken))
	{
		AuthorizationError(TEXT("Bad oauth response"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Authenticated"));

	AuthorizationChange.Broadcast(Authorization, EAuthorizationStatus::Authenticated);
}
*/