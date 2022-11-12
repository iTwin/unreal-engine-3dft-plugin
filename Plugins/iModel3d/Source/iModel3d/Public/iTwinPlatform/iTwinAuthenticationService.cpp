#include "iTwinAuthenticationService.h"

#include "Containers/Ticker.h"
#include "Misc/App.h"
#include "Misc/MessageDialog.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "JsonUtilities.h"

#include "Common/APIService.h"

// Authentication end points and credentials
struct FAuthenticationCredentials {
	static constexpr auto Hostname = TEXT("ims.bentley.com");
	static constexpr auto DeviceAuthorizationEndpoint = TEXT("/as/device_authz.oauth2");
	static constexpr auto TokenEndpoint = TEXT("/connect/token");
	static constexpr auto ClientId = TEXT("itwinxr-hololens-app");
	static constexpr auto Scope = TEXT("email openid profile organization forms:modify forms:read imodelhub issues:modify issues:read itwin-xr itwinjs projects:modify projects:read validation:modify validation:read");
	//static constexpr auto ClientId = TEXT("unreal-xr");
	//static constexpr auto Scope = TEXT("email openid profile organization imodelhub itwin-xr itwinjs projects:modify projects:read validation:modify validation:read");
};

void UiTwinAuthenticationService::InitiateAuthentication()
{
	FString RequestContent = FString::Printf(
		TEXT("client_id=%s&scope=%s"),
		FAuthenticationCredentials::ClientId, FAuthenticationCredentials::Scope);

	FAPIService::SendPostRequest(FAuthenticationCredentials::DeviceAuthorizationEndpoint, RequestContent, [this](TSharedPtr<FJsonObject> Response, const FString& ErrorMessage)
		{
			HandleOauthResponse(Response, ErrorMessage);
		}, FAuthenticationCredentials::Hostname);
}

void UiTwinAuthenticationService::CancelAllRequests()
{
	// Cancel the token request
	if (TickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
		TickerHandle.Reset();
	}

	AuthenticationChange.Broadcast({}, EAuthenticationStatus::Uninitialized);
}

void UiTwinAuthenticationService::AuthenticationError(const FString& ErrorMessage)
{
	UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);

	AuthenticationChange.Broadcast({}, EAuthenticationStatus::AuthenticationError);
}

void UiTwinAuthenticationService::HandleOauthResponse(TSharedPtr<FJsonObject> Response, const FString& ErrorMessage)
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
		AuthenticationError(ErrorCode);
		return;
	}

	FAuthentication Authentication;

	// No error at this point, handle the response now.
	// Retrieve the URL for verification and the device code.
	FString LaunchURL;
	if (!Response->TryGetStringField(TEXT("verification_uri_complete"), LaunchURL) ||
		!Response->TryGetStringField(TEXT("device_code"), Authentication.DeviceCode))
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid oauth response"));
		return;
	}

	// Read the poll interval
	double PollInterval = 3.0;
	Response->TryGetNumberField(TEXT("interval"), PollInterval);
	Authentication.AuthPollInterval = (float)PollInterval;

	FString Error;
	FPlatformProcess::LaunchURL(*LaunchURL, nullptr, &Error);
	if (Error.Len() != 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Error));
		return;
	}

	WaitAndRequestOauthToken(Authentication);
}

void UiTwinAuthenticationService::WaitAndRequestOauthToken(FAuthentication Authentication)
{
	AuthenticationChange.Broadcast(Authentication, EAuthenticationStatus::RequestedToken);

	TickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda(
		[this, Authentication](float Delta) -> bool
		{
			// Send the token request
			FString RequestContent = FString::Printf(
				TEXT("client_id=%s&device_code=%s&grant_type=%s"),
				FAuthenticationCredentials::ClientId, *Authentication.DeviceCode, TEXT("urn:ietf:params:oauth:grant-type:device_code"));
			FAPIService::SendPostRequest(FAuthenticationCredentials::TokenEndpoint, RequestContent, [this, Authentication](TSharedPtr<FJsonObject> Response, const FString& ErrorMessage)
				{
					HandleTokenResponse(Response, ErrorMessage, Authentication);
				}, FAuthenticationCredentials::Hostname);
			// Delete and stop the ticker (reset delegate and 'return false')
			// We have a valid http request now, and its handler may initiate a new timer, but at the monent we
			// should remove the current timer
			TickerHandle.Reset();
			return false;
		}), Authentication.AuthPollInterval);
}

void UiTwinAuthenticationService::HandleTokenResponse(TSharedPtr<FJsonObject> Response, const FString& ErrorMessage, FAuthentication Authentication)
{
	if (!ErrorMessage.IsEmpty())
	{
		AuthenticationError(ErrorMessage);
		return;
	}

	FString ErrorCode;
	Response->TryGetStringField(TEXT("error"), ErrorCode);
	if (ErrorCode == TEXT("authorization_pending"))
	{
		// User still have to authenticate in browser, repeat the request
		WaitAndRequestOauthToken(Authentication);
		return;
	}

	// Other errors:
	// - "slow_down" = polling too frequently
	// - "access_denied" = user declined the authorization
	if (!ErrorCode.IsEmpty())
	{
		AuthenticationError(ErrorCode);
		return;
	}

	if (!Response->TryGetStringField(TEXT("access_token"), Authentication.AuthToken))
	{
		AuthenticationError(TEXT("Bad oauth response"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Authenticated"));

	AuthenticationChange.Broadcast(Authentication, EAuthenticationStatus::Authenticated);
}