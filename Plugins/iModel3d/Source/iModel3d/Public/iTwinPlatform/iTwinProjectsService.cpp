/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "iTwinProjectsService.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "JsonUtilities.h"

#include "Common/APIService.h"

void UiTwinProjectsService::GetProjects(const FAuthentication& Authentication)
{
	FString RequestContent = TEXT("");
	FAPIService::SendGetRequest(TEXT("https://api.bentley.com/projects"), RequestContent, Authentication.AuthToken, [this](const FString& Response, const FString& ErrorMessage)
		{
			if (!ErrorMessage.IsEmpty())
			{
				UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
				return;
			}

			FProjectList ProjectList;
			FJsonObjectConverter::JsonObjectStringToUStruct<FProjectList>(Response, &ProjectList, 0, 0);
			ProjectListReceived.Broadcast(std::move(ProjectList));
		});
}
