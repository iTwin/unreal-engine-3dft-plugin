/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ProceduralMeshes.h"

FProceduralMeshes::FProceduralMeshes(AActor* InActor, bool InbIsInEditor)
	: Actor(InActor), bIsInEditor(InbIsInEditor)
{
}

void FProceduralMeshes::Update()
{
	UpdatePool();
	UpdateVisibility();
}

UCustomProcMeshComponent* FProceduralMeshes::Pop()
{
	BeingUsed++;
	auto Component = Pool[0];
	Pool.RemoveAt(0, 1, false);
	return Component;
}

void FProceduralMeshes::Push(UCustomProcMeshComponent* Component)
{
	Hide(Component);
}

size_t FProceduralMeshes::PoolNum()
{
	return Pool.Num();
}

void FProceduralMeshes::UpdatePool()
{
	while (BeingUsed + PoolNum() < PoolSize || PoolNum() < (PoolSize >> 8))
	{
		auto MeshComponent = NewObject<UCustomProcMeshComponent>(Actor);
		MeshComponent->SetMobility(EComponentMobility::Stationary);
		MeshComponent->bCastDynamicShadow = MeshComponent->CastShadow = false;
		MeshComponent->SetCastShadow(false);
		MeshComponent->AttachToComponent(Actor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

		auto EmptySection = MakeShared<FCustomProcMeshSection>();

		EmptySection->Indices.SetNum(3);
		EmptySection->Indices[0] = 0; EmptySection->Indices[1] = 1; EmptySection->Indices[2] = 2;
		EmptySection->Vertices.SetNumZeroed(3);
		EmptySection->Vertices[0].Position = { 0, 0, 0 };
		EmptySection->Vertices[1].Position = { 1, 0, 0 };
		EmptySection->Vertices[2].Position = { 1, 1, 0 };
		EmptySection->SectionLocalBox = FBox(FVector(0, 0, 0), FVector(1, 1, 0));

		MeshComponent->SetProcMeshSection(0, EmptySection);
		MeshComponent->SetMeshSectionVisible(0, false);
		MeshComponent->SetMaterial(0, nullptr);

		if (bIsInEditor)
		{
			// Mark components as transient in order for them not to be copied into play
			MeshComponent->SetFlags(MeshComponent->GetFlags() | EObjectFlags::RF_DuplicateTransient | EObjectFlags::RF_Transient);
		}

		MeshComponent->RegisterComponent();

		Pool.Push(MeshComponent);
		AllComponents.Push(MeshComponent);
	}
}

void FProceduralMeshes::UpdateVisibility()
{
	for (auto& Component : ComponentsToHide)
	{
		Component->bCastDynamicShadow = Component->CastShadow = false;
		Component->SetCastShadow(false);
		Component->ClearAllMeshSections();
		BeingUsed--;
		Pool.Push(Component);
	}

	for (auto& Component : ComponentsToShow)
	{
		Component->SetMeshSectionVisible(0, true);
	}

	ComponentsToShow.Empty();
	ComponentsToHide.Empty();
}

void FProceduralMeshes::Show(UCustomProcMeshComponent* Component)
{
	ComponentsToShow.Add(Component);
	//Component.MeshComponent->SetMeshSectionVisible(0, true);
}

void FProceduralMeshes::Hide(UCustomProcMeshComponent* Component)
{
	ComponentsToHide.Add(Component);
	//Component.MeshComponent->SetMeshSectionVisible(0, false);
}

void FProceduralMeshes::Reset()
{
	for (auto& Component : AllComponents)
	{
		Component->ClearAllMeshSections();
		Component->UnregisterComponent();
	}
}