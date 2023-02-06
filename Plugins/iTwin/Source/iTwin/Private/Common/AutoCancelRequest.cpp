/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "AutoCancelRequest.h"

FAutoCancelRequest::~FAutoCancelRequest()
{
	Reset();
}

FAutoCancelRequest::FRequestRef FAutoCancelRequest::operator =(FRequestRef InRequest)
{
	Reset();
	Requests.Push(InRequest);
	return InRequest;
}

void FAutoCancelRequest::Reset()
{
	if (Requests.Num())
	{
		for (int i = 0; i < Requests.Num(); i++)
		{
			if (Requests[i]->GetStatus() == EHttpRequestStatus::NotStarted || Requests[i]->GetStatus() == EHttpRequestStatus::Processing)
			{
				Requests[i]->CancelRequest();
			}
		}
		Requests.Reset();
	}
}
