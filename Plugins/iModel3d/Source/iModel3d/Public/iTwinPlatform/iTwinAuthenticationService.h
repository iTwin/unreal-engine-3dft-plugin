#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "iTwinAuthenticationService.generated.h"

class FJsonObject;

UENUM(BlueprintType)
enum class EAuthenticationStatus : uint8
{
	Uninitialized,
	AuthenticationError,
	RequestedToken,
	Authenticated,
};

USTRUCT(BlueprintType)
struct FAuthentication
{
	GENERATED_BODY()

	FString DeviceCode;

	FString AuthToken;

	float AuthPollInterval = 3.f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAuthenticationChange, FAuthentication, Authentication, EAuthenticationStatus, State);

UCLASS(BlueprintType)
class IMODEL3D_API UiTwinAuthenticationService : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
		FOnAuthenticationChange AuthenticationChange;

	UFUNCTION(BlueprintCallable)
		void InitiateAuthentication();

	UFUNCTION(BlueprintCallable)
		void CancelAllRequests();

private:
	void HandleOauthResponse(TSharedPtr<FJsonObject> Response, const FString& ErrorMessage);

	void HandleTokenResponse(TSharedPtr<FJsonObject> Response, const FString& ErrorMessage, FAuthentication Authentication);

	void WaitAndRequestOauthToken(FAuthentication Authentication);

	void AuthenticationError(const FString& ErrorMessage);

	FTSTicker::FDelegateHandle TickerHandle;
};

