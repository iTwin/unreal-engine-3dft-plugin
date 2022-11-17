/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Actor.h"
#include "CustomProcMeshComponent.h"

class FProceduralMeshes
{
public:
	FProceduralMeshes(AActor* InActor, bool bIsInEditor = false);

	UCustomProcMeshComponent* Pop();

	size_t PoolNum();

	void Push(UCustomProcMeshComponent* Component);

	void Update();

	void Show(UCustomProcMeshComponent* Component);

	void Hide(UCustomProcMeshComponent* Component);

private:
	void UpdatePool();

	void UpdateVisibility();

	AActor* Actor;
	const int PoolSize = 1024;
	int BeingUsed = 0;
	TArray<UCustomProcMeshComponent*> Pool;
	TArray<UCustomProcMeshComponent*> ComponentsToShow;
	TArray<UCustomProcMeshComponent*> ComponentsToHide;
	bool bIsInEditor;
};
