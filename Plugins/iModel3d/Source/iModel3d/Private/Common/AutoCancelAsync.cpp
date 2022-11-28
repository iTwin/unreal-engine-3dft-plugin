/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "AutoCancelAsync.h"

FAutoCancelAsync::~FAutoCancelAsync()
{
	Reset();
}

FTSTicker::FDelegateHandle& FAutoCancelAsync::operator =(FTSTicker::FDelegateHandle& Handle)
{
	Reset();
	DelegateHandle = Handle;
	return Handle;
}

FTSTicker::FDelegateHandle& FAutoCancelAsync::operator =(FTSTicker::FDelegateHandle&& Handle)
{
	Reset();
	DelegateHandle = Handle;
	return DelegateHandle;
}

void FAutoCancelAsync::Reset()
{
	if (DelegateHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(DelegateHandle);
		DelegateHandle.Reset();
	}
}
