/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"

struct FAutoCancelTicker
{
public:
	~FAutoCancelTicker();

	void Reset();

	FTSTicker::FDelegateHandle& operator =(FTSTicker::FDelegateHandle &Handle);

	FTSTicker::FDelegateHandle& operator =(FTSTicker::FDelegateHandle &&Handle);

private:
	FTSTicker::FDelegateHandle DelegateHandle;
};