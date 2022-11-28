/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "CoreMinimal.h"

#include "HttpModule.h"

struct FAutoCancelRequest
{
public:
	using FRequestRef = TSharedRef<IHttpRequest, ESPMode::ThreadSafe>;
public:
	~FAutoCancelRequest();

	void Reset();

	FRequestRef operator =(FRequestRef InRequest);

private:
	TArray<FRequestRef> Requests;
};