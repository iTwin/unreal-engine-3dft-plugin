/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "AutoCancelTicker.h"

FAutoCancelTicker::~FAutoCancelTicker()
{
	Reset();
}

FTSTicker::FDelegateHandle& FAutoCancelTicker::operator =(FTSTicker::FDelegateHandle& Handle)
{
	Reset();
	DelegateHandle = Handle;
	return Handle;
}

FTSTicker::FDelegateHandle& FAutoCancelTicker::operator =(FTSTicker::FDelegateHandle&& Handle)
{
	Reset();
	DelegateHandle = Handle;
	return DelegateHandle;
}

void FAutoCancelTicker::Reset()
{
	if (DelegateHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(DelegateHandle);
		DelegateHandle.Reset();
	}
}
