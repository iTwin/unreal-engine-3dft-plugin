/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"

struct FAutoCancelAsync
{
public:
	~FAutoCancelAsync();

	FTSTicker::FDelegateHandle& operator =(FTSTicker::FDelegateHandle &Handle);
	FTSTicker::FDelegateHandle& operator =(FTSTicker::FDelegateHandle &&Handle);

private:
	void Reset();

	FTSTicker::FDelegateHandle DelegateHandle;
};